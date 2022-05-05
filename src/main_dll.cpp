#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <ztd/text.hpp>

#define WIN32_LEAN_AND_MEAN
#include <shlobj.h>
#include <wil/stl.h>
#include <wil/win32_helpers.h>
#include <windows.h>

#define MiniDumpWriteDump MiniDumpWriteDump_
#include <minidumpapiset.h>
#undef MiniDumpWriteDump

#include "config.h"
#include "game_info.h"
#include "lua_exec.h"
#include "wil_extra.h"
#include "x86_64.h"

// TODO: Remove after init fix.
#include <thread>

namespace fs = std::filesystem;

using MiniDumpWriteDumpProc = decltype(&MiniDumpWriteDump_);

using GameFrameProc = std::uint64_t(__cdecl*)(void* rcx);

MiniDumpWriteDumpProc writeDumpProc = nullptr;

GameFrameProc* frameProcPtr = nullptr;
GameFrameProc frameProc = nullptr;

std::optional<Config> config;
std::optional<GameInfo> gameInfo;

std::uint64_t moduleAddress = 0;

std::optional<std::uintptr_t> followPointerChain(
    std::uintptr_t start, const std::vector<std::uintptr_t>& offsets) {
    std::uintptr_t current = start;

    for (auto it = offsets.cbegin(); it != offsets.cend(); ++it) {
        if (current == 0) return {};

        if (it != offsets.cend() - 1) {
            current = *reinterpret_cast<std::uintptr_t*>(current + *it);
        } else {
            current += *it;
        }
    }

    return current;
}

std::uint64_t __cdecl frameHook(void* rcx) {
    ExecuteLUA();
    return frameProc(rcx);
}

LONG WINAPI crashDumpHandler(PEXCEPTION_POINTERS exceptionPointers) {
    HANDLE file = CreateFileW(L"CrashDump.dmp", GENERIC_READ | GENERIC_WRITE, 0,
                              NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file != INVALID_HANDLE_VALUE) {
        MINIDUMP_EXCEPTION_INFORMATION mdei;

        mdei.ThreadId = GetCurrentThreadId();
        mdei.ExceptionPointers = exceptionPointers;
        mdei.ClientPointers = TRUE;

        (*writeDumpProc)(GetCurrentProcess(), GetCurrentProcessId(), file,
                         MiniDumpNormal, &mdei, 0, 0);

        CloseHandle(file);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

bool hookGame() {
    static_assert(sizeof(std::uint64_t) == sizeof(std::uintptr_t));

    const std::vector<std::uintptr_t> frameProcOffsets{0x3E8, 0x0, 0x20};
    const std::vector<std::uintptr_t> graphicsProcOffsets{0x2D8};

    std::uintptr_t pointerStruct =
        moduleAddress + gameInfo->pointerStructOffset;

    if (auto ptr = followPointerChain(pointerStruct, frameProcOffsets)) {
        frameProcPtr = reinterpret_cast<GameFrameProc*>(*ptr);
    } else {
        return false;
    }

    if (*frameProcPtr == nullptr) return false;

    DWORD originalProt = 0;
    VirtualProtect(frameProcPtr, sizeof(frameProcPtr), PAGE_READWRITE,
                   &originalProt);
    frameProc = *frameProcPtr;
    *frameProcPtr = frameHook;
    VirtualProtect(frameProcPtr, sizeof(frameProcPtr), originalProt,
                   &originalProt);

    SetUnhandledExceptionFilter(crashDumpHandler);

    return true;
}

DWORD WINAPI entry(LPVOID lpParameter) {
    (void)lpParameter;

    std::wstring modulePathStr;
    wil::GetModuleFileNameW(nullptr, modulePathStr);

    fs::path modulePath = fs::path{modulePathStr};
    fs::path moduleDir = modulePath.parent_path();
    std::wstring moduleNameW = moduleDir.filename();

    std::string moduleName = ztd::text::transcode(
        moduleNameW, ztd::text::wide_utf16, ztd::text::compat_utf8,
        ztd::text::replacement_handler);

    try {
        config = Config::load("LuaBackend.toml");
        auto entry = config->gameInfo(moduleName);
        if (entry) {
            gameInfo = *entry;
        } else {
            return 0;
        }

        moduleAddress = (std::uint64_t)GetModuleHandleW(nullptr);
        std::uint64_t baseAddress = moduleAddress + gameInfo->baseAddress;

        std::vector<fs::path> scriptPaths;
        for (const auto& path : gameInfo->scriptPaths) {
            if (path.relative) {
                PWSTR khDocsRootStr;
                SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr,
                                     &khDocsRootStr);
                fs::path khDocsRoot = fs::path{khDocsRootStr} /
                                      L"KINGDOM HEARTS HD 1.5+2.5 ReMIX";

                fs::path gameScriptsPath = fs::path{khDocsRoot} / path.str;
                if (fs::exists(gameScriptsPath)) {
                    scriptPaths.push_back(gameScriptsPath);
                }
            } else {
                fs::path gameScriptsPath = fs::path{path.str};
                if (fs::exists(gameScriptsPath)) {
                    scriptPaths.push_back(gameScriptsPath);
                }
            }
        }

        if (!scriptPaths.empty()) {
            AllocConsole();
            FILE* f;
            freopen_s(&f, "CONOUT$", "w", stdout);

            if (EntryLUA(GetCurrentProcessId(), GetCurrentProcess(),
                         baseAddress, std::move(scriptPaths)) == 0) {
                // TODO: Hook after game initialization is done.
                while (!hookGame()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(16));
                }
            } else {
                std::cout << "Failed to initialize internal LuaBackend!"
                          << std::endl;
            }
        }
    } catch (std::exception& e) {
        std::string msg = "entry exception: " + std::string(e.what()) +
                          "\n\nScripts failed to load.";
        std::wstring wmsg = ztd::text::transcode(
            msg, ztd::text::compat_utf8, ztd::text::wide_utf16,
            ztd::text::replacement_handler);
        MessageBoxW(NULL, wmsg.c_str(), L"LuaBackendHook",
                    MB_ICONERROR | MB_OK);
    }

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    (void)hinstDLL;
    (void)lpReserved;
    static HMODULE dbgHelp = nullptr;

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            std::wstring systemDirectoryStr;
            wil::GetSystemDirectoryW(systemDirectoryStr);
            fs::path dllPath = fs::path{systemDirectoryStr} / L"DBGHELP.dll";

            dbgHelp = LoadLibraryW(dllPath.wstring().c_str());
            writeDumpProc = (MiniDumpWriteDumpProc)GetProcAddress(
                dbgHelp, "MiniDumpWriteDump");

            if (CreateThread(nullptr, 0, entry, nullptr, 0, nullptr) ==
                nullptr) {
                return FALSE;
            }

            break;
        }
        case DLL_PROCESS_DETACH: {
            FreeLibrary(dbgHelp);
            break;
        }
        default:
            break;
    }

    return TRUE;
}

extern "C" __declspec(dllexport) BOOL WINAPI
    MiniDumpWriteDump(HANDLE hProcess, DWORD ProcessId, HANDLE hFile,
                      MINIDUMP_TYPE DumpType,
                      PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
                      PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                      PMINIDUMP_CALLBACK_INFORMATION CallbackParam) {
    return writeDumpProc(hProcess, ProcessId, hFile, DumpType, ExceptionParam,
                         UserStreamParam, CallbackParam);
}
