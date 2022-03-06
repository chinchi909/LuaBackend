#include <chrono>
#include <iostream>
#include <filesystem>
#include <string_view>
#include <vector>

#include <unordered_map>
#include <optional>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>

#define MiniDumpWriteDump MiniDumpWriteDump_Original
#include <minidumpapiset.h>
#undef MiniDumpWriteDump

#include <MemoryLib.h>
#include <LuaBackend.h>
#include <mIni/ini.h>

#include "x86_64.h"
#include "game_info.h"

// TODO: Remove after init fix.
#include <thread>

const std::unordered_map<std::string_view, GameInfo> gameInfos{
    { "KINGDOM HEARTS FINAL MIX.exe", { 0x22B7280, 0x3A0606, 0, "kh1" } },
    { "KINGDOM HEARTS Re_Chain of Memories.exe", { 0xBF7A80, 0x4E4660, 0, "recom" } },
    { "KINGDOM HEARTS II FINAL MIX.exe", { 0x89E9A0, 0x56454E, 0x12B350, "kh2" } },
    { "KINGDOM HEARTS Birth by Sleep FINAL MIX.exe", { 0x110B5970, 0x60E334, 0, "bbs" } },
};

namespace fs = std::filesystem;

using MiniDumpWriteDumpProc = decltype(&MiniDumpWriteDump_Original);

using GameFrameProc = std::uint64_t(__cdecl*)(void* rcx);

MiniDumpWriteDumpProc writeDumpProc = nullptr;

GameFrameProc* frameProcPtr = nullptr;
GameFrameProc frameProc = nullptr;

std::optional<GameInfo> gameInfo{};
std::uint64_t moduleAddress;

extern "C"
{
    using namespace std;
    using namespace mINI;

    bool _funcOneState = true;
    bool _funcTwoState = true;
    bool _funcThreeState = true;

    string _scrPath;

    bool _showConsole = false;
    bool _requestedReset = false;

    LuaBackend* _backend = nullptr;

    chrono::high_resolution_clock::time_point _sClock;
    chrono::high_resolution_clock::time_point _msClock;

    void EnterWait()
    {
        string _output;
        getline(std::cin, _output);
        return;
    }

    void ResetLUA()
    {
        printf("\n");
        ConsoleLib::MessageOutput("Reloading...\n\n", 0);
        _backend = new LuaBackend(_scrPath.c_str(), MemoryLib::ExecAddress + MemoryLib::BaseAddress);

        if (_backend->loadedScripts.size() == 0)
            ConsoleLib::MessageOutput("No scripts found! Reload halted!\n\n", 3);

        ConsoleLib::MessageOutput("Executing initialization event handlers...\n\n", 0);

        for (auto _script : _backend->loadedScripts)
            if (_script->initFunction)
            {
                auto _result = _script->initFunction();

                if (!_result.valid())
                {
                    sol::error _err = _result;
                    ConsoleLib::MessageOutput(_err.what(), 3);
                    printf("\n\n");
                }
            }

        ConsoleLib::MessageOutput("Reload complete!\n\n", 1);

        _msClock = chrono::high_resolution_clock::now();
        _sClock = chrono::high_resolution_clock::now();

        _requestedReset = false;
    }

    int __declspec(dllexport) EntryLUA(int ProcessID, HANDLE ProcessH, uint64_t TargetAddress, const char* ScriptPath)
    {
        ShowWindow(GetConsoleWindow(), SW_HIDE);

        cout << "======================================" << "\n";
        cout << "======= LuaBackendHook | v1.2.1 ======" << "\n";
        cout << "====== Copyright 2021 - TopazTK ======" << "\n";
        cout << "======================================" << "\n";
        cout << "=== Compatible with LuaEngine v5.0 ===" << "\n";
        cout << "========== Embedded Version ==========" << "\n";
        cout << "======================================" << "\n\n";

        ConsoleLib::MessageOutput("Initializing LuaEngine v5.0...\n\n", 0);
        _scrPath = string(ScriptPath);

        MemoryLib::ExternProcess(ProcessID, ProcessH, TargetAddress);

        _backend = new LuaBackend(ScriptPath, MemoryLib::ExecAddress + TargetAddress);
        _backend->frameLimit = 16;

        if (_backend->loadedScripts.size() == 0)
        {
            ConsoleLib::MessageOutput("No scripts were found! Initialization halted!\n\n", 3);
            return -1;
        }

        ConsoleLib::MessageOutput("Executing initialization event handlers...\n\n", 0);

        for (auto _script : _backend->loadedScripts)
            if (_script->initFunction)
            {
                auto _result = _script->initFunction();

                if (!_result.valid())
                {
                    sol::error _err = _result;
                    ConsoleLib::MessageOutput(_err.what(), 3);
                    printf("\n\n");
                }
            }

        ConsoleLib::MessageOutput("Initialization complete!\n", 1);
        ConsoleLib::MessageOutput("Press 'F1' to reload all scripts, press 'F2' to toggle the console, press 'F3' to set execution frequency.\n\n", 0);

        _msClock = chrono::high_resolution_clock::now();
        _sClock = chrono::high_resolution_clock::now();

        return 0;
    }

    void __declspec(dllexport) ExecuteLUA()
    {
        if (_requestedReset == false)
        {
            auto _currTime = chrono::high_resolution_clock::now();
            auto _msTime = std::chrono::duration_cast<std::chrono::milliseconds>(_currTime - _msClock).count();
            auto _sTime = std::chrono::duration_cast<std::chrono::milliseconds>(_currTime - _sClock).count();

            if (GetKeyState(VK_F3) & 0x8000 && _funcThreeState)
            {
                switch (_backend->frameLimit)
                {
                case 16:
                    _backend->frameLimit = 8;
                    ConsoleLib::MessageOutput("Frequency set to 120Hz.\n", 0);
                    break;
                case 8:
                    _backend->frameLimit = 4;
                    ConsoleLib::MessageOutput("Frequency set to 240Hz.\n", 0);
                    break;
                case 4:
                    _backend->frameLimit = 16;
                    ConsoleLib::MessageOutput("Frequency set to 60Hz.\n", 0);
                    break;
                }

                _sTime = 0;
                _funcThreeState = false;
                _sClock = chrono::high_resolution_clock::now();
            }
            if (GetKeyState(VK_F2) & 0x8000 && _funcTwoState)
            {
                if (_showConsole)
                {
                    ShowWindow(GetConsoleWindow(), SW_HIDE);
                    _showConsole = false;
                }

                else
                {
                    ShowWindow(GetConsoleWindow(), SW_RESTORE);
                    _showConsole = true;
                }

                _sTime = 0;
                _funcTwoState = false;
                _sClock = chrono::high_resolution_clock::now();
            }
            if (GetKeyState(VK_F1) & 0x8000 && _funcOneState)
            {
                _requestedReset = true;

                _sTime = 0;
                _funcOneState = false;
                _sClock = chrono::high_resolution_clock::now();
            }

            for (int i = 0; i < _backend->loadedScripts.size(); i++)
            {
                auto _script = _backend->loadedScripts[i];

                if (_script->frameFunction)
                {
                    auto _result = _script->frameFunction();

                    if (!_result.valid())
                    {
                        sol::error _err = _result;
                        ConsoleLib::MessageOutput(_err.what(), 3);
                        printf("\n\n");

                        _backend->loadedScripts.erase(_backend->loadedScripts.begin() + i);
                    }
                }
            }

            _msClock = chrono::high_resolution_clock::now();

            if (_sTime > 250)
            {
                _funcOneState = true;
                _funcTwoState = true;
                _funcThreeState = true;
                _sClock = chrono::high_resolution_clock::now();
            }
        }

        else
            ResetLUA();
    }

    bool __declspec(dllexport) CheckLUA()
    {
        auto _int = MemoryLib::ReadInt(0);

        if (_int == 0)
            return false;

        return true;
    }

    int __declspec(dllexport) VersionLUA()
    {
        return 128;
    }
}

std::optional<std::uintptr_t> followPointerChain(std::uintptr_t start, const std::vector<std::uintptr_t>& offsets) {
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

LONG WINAPI crashHandler(PEXCEPTION_POINTERS exceptionInfo) {
    PCONTEXT ctx = exceptionInfo->ContextRecord;

    if (ctx->Rip == moduleAddress + gameInfo->hookAddress) {
        ctx->Rax = *reinterpret_cast<std::uintptr_t*>(ctx->Rcx);
        ctx->Rip += 3;

        ExecuteLUA();

        return EXCEPTION_CONTINUE_EXECUTION;
    } else if (exceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        HANDLE file = CreateFileA("CrashDump.dmp", GENERIC_READ | GENERIC_WRITE, 0,
            NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if (file != INVALID_HANDLE_VALUE) {
            MINIDUMP_EXCEPTION_INFORMATION mdei;

            mdei.ThreadId = GetCurrentThreadId();
            mdei.ExceptionPointers = exceptionInfo;
            mdei.ClientPointers = TRUE;

            (*writeDumpProc)(GetCurrentProcess(), GetCurrentProcessId(),
                file, MiniDumpNormal, (exceptionInfo != NULL) ? &mdei : 0, 0, 0);

            CloseHandle(file);
        }
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

bool hookGame() {
    static_assert(sizeof(std::uint64_t) == sizeof(std::uintptr_t));

    void* hookAddress = reinterpret_cast<void*>(moduleAddress + gameInfo->hookAddress);
    std::uint8_t hook[] = { 0xCC, 0x90, 0x90 };

    DWORD originalProt = 0;
    VirtualProtect(hookAddress, 3, PAGE_READWRITE, &originalProt);
    std::memcpy(hookAddress, hook, sizeof(hook));
    VirtualProtect(hookAddress, 3, originalProt, &originalProt);

    AddVectoredExceptionHandler(0, crashHandler);

    return true;
}

DWORD WINAPI entry(LPVOID lpParameter) {
    char modulePath[MAX_PATH];
    GetModuleFileNameA(nullptr, modulePath, MAX_PATH);

    std::string_view moduleName = modulePath;
    std::size_t pos = moduleName.rfind("\\");
    if (pos != std::string_view::npos) {
        moduleName.remove_prefix(pos + 1);
    }

    auto entry = gameInfos.find(moduleName);
    if (entry != gameInfos.end()) {
        gameInfo = entry->second;
    } else {
        return 0;
    }

    moduleAddress = (uint64_t)GetModuleHandleA(nullptr);
    std::uint64_t baseAddress = moduleAddress + gameInfo->baseAddress;
    char scriptsPath[MAX_PATH];
    SHGetFolderPathA(0, CSIDL_MYDOCUMENTS, nullptr, 0, scriptsPath);
    std::strcat(scriptsPath, "\\KINGDOM HEARTS HD 1.5+2.5 ReMIX\\scripts");

    fs::path gameScriptsPath = fs::path{scriptsPath} / gameInfo->scriptsPath;

    if (fs::exists(gameScriptsPath)) {
        AllocConsole();
        std::freopen("CONOUT$", "w", stdout);

        if (EntryLUA(GetCurrentProcessId(), GetCurrentProcess(), baseAddress, gameScriptsPath.u8string().c_str()) == 0) {
            // TODO: Hook after game initialization is done.
            while (!hookGame()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
            }
        } else {
            std::cout << "Failed to initialize internal LuaBackend!" << std::endl;
        }
    }

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    static HMODULE dbgHelp = nullptr;

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            char dllPath[MAX_PATH];
            GetSystemDirectoryA(dllPath, MAX_PATH);
            std::strcat(dllPath, "\\DBGHELP.dll");
            dbgHelp = LoadLibraryA(dllPath);
            writeDumpProc = (MiniDumpWriteDumpProc)GetProcAddress(dbgHelp, "MiniDumpWriteDump");

            if (CreateThread(nullptr, 0, entry, nullptr, 0, nullptr) == nullptr) {
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

extern "C" __declspec(dllexport) BOOL WINAPI MiniDumpWriteDump(
    HANDLE hProcess,
    DWORD ProcessId,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    PMINIDUMP_CALLBACK_INFORMATION CallbackParam
) {
    return writeDumpProc(hProcess, ProcessId, hFile, DumpType, ExceptionParam, UserStreamParam, CallbackParam);
}
