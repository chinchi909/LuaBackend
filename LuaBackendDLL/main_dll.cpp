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

#include <MemoryLib.h>
#include <LuaBackend.h>
#include <mIni/ini.h>

#include "x86_64.h"
#include "game_info.h"

const std::unordered_map<std::string_view, GameInfo> gameInfos{
    { "KINGDOM HEARTS FINAL MIX.exe", { { 0x10814C, 14 }, 0x3A0606, "kh1" } },
    { "KINGDOM HEARTS Re_Chain of Memories.exe", { { 0x2E189C, 14 }, 0x4E4660, "recom" } },
    { "KINGDOM HEARTS II FINAL MIX.exe", { { 0x12B26C, 14 }, 0x56450E, "kh2" } },
    { "KINGDOM HEARTS Birth by Sleep FINAL MIX.exe", { { 0x4EE23C, 14 }, 0x60E334, "bbs" } },
};

namespace fs = std::filesystem;

using DirectInput8CreateProc = HRESULT(WINAPI*)(HINSTANCE hinst, DWORD dwVersion, LPCVOID riidltf, LPVOID* ppvOut, LPVOID punkOuter);

DirectInput8CreateProc createProc = nullptr;

std::optional<GameInfo> gameInfo{};

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
		cout << "========= LuaBackend | v1.30 =========" << "\n";
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

extern "C" void onFrame() {
    ExecuteLUA();
}

void hookGame(uint64_t moduleAddress) {
    static_assert(sizeof(uint64_t) == sizeof(uintptr_t));

    std::vector<uint8_t> relocated(gameInfo->hookInfo.size);
    uintptr_t hookStart = moduleAddress + gameInfo->hookInfo.start;
    uintptr_t hookEnd = hookStart + relocated.size();

    DWORD originalProt = 0;
    VirtualProtect((void*)hookStart, relocated.size(), PAGE_EXECUTE_READWRITE, &originalProt);
    std::memcpy(relocated.data(), (void*)hookStart, relocated.size());

    std::vector<uint8_t> func{
        PushRbx,
        PUSHF_1, PUSHF_2,
        0x48, 0x89, 0xE3,       // mov rbx, rsp
        0x48, 0x83, 0xE4, 0xF0, // and rsp, -0x10
        PushRax,
        PushRcx,
        PushRdx,
        PushR8_1, PushR8_2,
        PushR9_1, PushR9_2,
        PushR10_1, PushR10_2,
        PushR11_1, PushR11_2,
        0x48, 0x83, 0xEC, 0x18, // sub rsp, 0x18
        CALL((uintptr_t)&onFrame),
        0x48, 0x83, 0xC4, 0x18, // add rsp, 0x18
        PopR11_1, PopR11_2,
        PopR10_1, PopR10_2,
        PopR9_1, PopR9_2,
        PopR8_1, PopR8_2,
        PopRdx,
        PopRcx,
        PopRax,
        0x48, 0x89, 0xDC,       // mov rsp, rbx
        POPF_1, POPF_2,
        PopRbx,
    };

    uint8_t funcReturn[] = {
        JUMP_TO(hookEnd),
    };

    func.insert(func.end(), relocated.begin(), relocated.end());
    func.insert(func.end(), std::begin(funcReturn), std::end(funcReturn));

    void* funcPtr = VirtualAlloc(nullptr, func.size(), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    uintptr_t funcAddress = (uintptr_t)funcPtr;

    std::vector<uint8_t> patch{
        JUMP_TO(funcAddress),
    };

    while (patch.size() < gameInfo->hookInfo.size) {
        patch.push_back(NOP);
    }

    std::memcpy((void*)funcAddress, func.data(), func.size());
    std::memcpy((void*)hookStart, patch.data(), patch.size());

    VirtualProtect((void*)hookStart, relocated.size(), originalProt, &originalProt);
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

    uint64_t moduleAddress = (uint64_t)GetModuleHandleA(nullptr);
    uint64_t baseAddress = moduleAddress + gameInfo->baseAddress;
    char scriptsPath[MAX_PATH];
    SHGetFolderPathA(0, CSIDL_MYDOCUMENTS, nullptr, 0, scriptsPath);
    std::strcat(scriptsPath, "\\KINGDOM HEARTS HD 1.5+2.5 ReMIX\\scripts");

    fs::path gameScriptsPath = fs::path{scriptsPath} / gameInfo->scriptsPath;

    if (fs::exists(gameScriptsPath)) {
        AllocConsole();
        std::freopen("CONOUT$", "w", stdout);

        if (EntryLUA(GetCurrentProcessId(), GetCurrentProcess(), baseAddress, gameScriptsPath.u8string().c_str()) == 0) {
            hookGame(moduleAddress);
        } else {
            std::cout << "Failed to initialize internal LuaBackend!" << std::endl;    
        }
    }

    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    static HMODULE dinput8 = nullptr;

    switch (fdwReason) {
        case DLL_PROCESS_ATTACH: {
            char dllPath[MAX_PATH];
            GetSystemDirectoryA(dllPath, MAX_PATH);
            std::strcat(dllPath, "\\DINPUT8.dll");
            dinput8 = LoadLibraryA(dllPath);
            createProc = (DirectInput8CreateProc)GetProcAddress(dinput8, "DirectInput8Create");

            if (CreateThread(nullptr, 0, entry, nullptr, 0, nullptr) == nullptr) {
                return FALSE;
            }

            break;
        }
        case DLL_PROCESS_DETACH: {
            FreeLibrary(dinput8);
            break;
        }
        default:
            break;
    }

    return TRUE;
}

extern "C" __declspec(dllexport) HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, LPCVOID riidltf, LPVOID* ppvOut, LPVOID punkOuter) {
    return createProc(hinst, dwVersion, riidltf, ppvOut, punkOuter);
}
