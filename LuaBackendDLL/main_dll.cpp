#include <chrono>
#include <iostream>
#include <filesystem>
#include <string_view>
#include <vector>

#include <condition_variable>
#include <thread>
#include <mutex>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>

#include <MemoryLib.h>
#include <LuaBackend.h>
#include <mIni/ini.h>

#include "x86_64.h"

using DirectInput8CreateProc = HRESULT(WINAPI*)(HINSTANCE hinst, DWORD dwVersion, LPCVOID riidltf, LPVOID* ppvOut, LPVOID punkOuter);

DirectInput8CreateProc createProc = nullptr;

std::mutex m;
std::condition_variable cv;
bool doScriptsFrame = false;
bool scriptsFrameDone = false;

std::thread luaThread;

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

void LuaThread() {
    while (true) {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [] { return doScriptsFrame; });

        ExecuteLUA();
        doScriptsFrame = false;
        scriptsFrameDone = true;

        lk.unlock();
        cv.notify_one();
    }
}

extern "C" void onFrame() {
    {
        std::lock_guard<std::mutex> lk(m);
        doScriptsFrame = true;
        scriptsFrameDone = false;
    }
    cv.notify_one();

    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [] { return scriptsFrameDone; });
    }
}

void hookGame(uint64_t moduleAddress) {
    static_assert(sizeof(uint64_t) == sizeof(uintptr_t));

    uint8_t relocated[0xF];
    uintptr_t hookStart = moduleAddress + 0x14C845;
    uintptr_t hookEnd = hookStart + sizeof(relocated);

    DWORD originalProt = 0;
    VirtualProtect((void*)hookStart, sizeof(relocated), PAGE_EXECUTE_READWRITE, &originalProt);
    std::memcpy((void*)relocated, (void*)hookStart, sizeof(relocated));

    uint8_t func[] = {
        PUSHF_1, PUSHF_2,
        PushRax,
        PushRcx,
        PushRdx,
        PushR8_1, PushR8_2,
        PushR9_1, PushR9_2,
        PushR10_1, PushR10_2,
        PushR11_1, PushR11_2,
        0x48, 0x83, 0xEC, 0x06, // sub rsp, 0x06
        CALL((uintptr_t)&onFrame),
        0x48, 0x83, 0xC4, 0x06, // add rsp, 0x06
        PopR11_1, PopR11_2,
        PopR10_1, PopR10_2,
        PopR9_1, PopR9_2,
        PopR8_1, PopR8_2,
        PopRdx,
        PopRcx,
        PopRax,
        POPF_1, POPF_2,
        JUMP_TO(hookEnd),
    };

    uint64_t funcSize = sizeof(relocated) + sizeof(func);
    void* funcPtr = VirtualAlloc(nullptr, funcSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    uintptr_t funcAddress = (uintptr_t)funcPtr;

    uint8_t patch[] = {
        JUMP_TO(funcAddress),
        NOP,
    };

    static_assert(sizeof(relocated) == sizeof(patch));
    std::memcpy((void*)hookStart, (void*)patch, sizeof(patch));

    std::memcpy((void*)funcAddress, relocated, sizeof(relocated));
    std::memcpy((void*)(funcAddress + sizeof(relocated)), func, sizeof(func));

    VirtualProtect((void*)hookStart, sizeof(relocated), originalProt, &originalProt);
}

DWORD WINAPI entry(LPVOID lpParameter) {
    char modulePath[MAX_PATH];
    GetModuleFileNameA(nullptr, modulePath, MAX_PATH);

    std::string_view moduleName = modulePath;
    std::size_t pos = moduleName.rfind("\\");
    if (pos != std::string_view::npos) {
        moduleName.remove_prefix(pos + 1);
    }

    if (moduleName != "KINGDOM HEARTS II FINAL MIX.exe") return 0;

    AllocConsole();
    std::freopen("CONOUT$", "w", stdout);

    uint64_t moduleAddress = (uint64_t)GetModuleHandleA(nullptr);
    uint64_t baseAddress = moduleAddress + 0x56450E;
    char scriptsPath[MAX_PATH];
    SHGetFolderPathA(0, CSIDL_MYDOCUMENTS, nullptr, 0, scriptsPath);
    std::strcat(scriptsPath, "\\KINGDOM HEARTS HD 1.5+2.5 ReMIX\\scripts");

    luaThread = std::thread(LuaThread);
    luaThread.detach();

    if (std::filesystem::exists(scriptsPath)) {
        if (EntryLUA(GetCurrentProcessId(), GetCurrentProcess(), baseAddress, scriptsPath) == 0) {
            hookGame(moduleAddress);
        } else {
            std::cout << "Failed to initialize internal LuaBackend!" << std::endl;    
        }
    } else {
        std::cout << "Scripts directory does not exist! Expected at: " << scriptsPath << std::endl;
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
