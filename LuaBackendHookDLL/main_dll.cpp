#include <chrono>
#include <iostream>
#include <filesystem>

#include <windows.h>

#include <MemoryLib.h>
#include <LuaBackend.h>
#include <mIni/ini.h>
#include <x86.h>

extern "C"
{
	using namespace std;
	using namespace mINI;

	const uint8_t _relocatedCode[] = {
		0x39, 0xB9, 0x68, 0x12, 0x00, 0x00, // cmp [rcx+00001268],edi
		0x0F, 0x4D, 0xF8,                   // cmovge edi,eax
		0x8B, 0xC7,                         // mov eax,edi
		0x48, 0x83, 0xC4, 0x40,             // add rsp,40
	};

	bool _funcOneState = true;
	bool _funcTwoState = true; 
	bool _funcThreeState = true;

	int _processID = 0;
	HANDLE _processH = nullptr;
	uint64_t _targetAddress = 0;
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
		AllocConsole();
		freopen("CONOUT$", "w", stdout);

		ShowWindow(GetConsoleWindow(), SW_HIDE);

		cout << "======================================" << "\n";
		cout << "========= LuaBackend | v1.30 =========" << "\n";
		cout << "====== Copyright 2021 - TopazTK ======" << "\n";
		cout << "======================================" << "\n";
		cout << "=== Compatible with LuaEngine v4.2 ===" << "\n";
		cout << "========== Embedded Version ==========" << "\n";
		cout << "======================================" << "\n\n";

		ConsoleLib::MessageOutput("Initializing LuaEngine v4.2...\n\n", 0);
		_processID = ProcessID;
		_processH = ProcessH;
		_targetAddress = TargetAddress;
		if (_scrPath.empty())
		{
			_scrPath = string(ScriptPath);
		}

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

	void OnFrame()
	{
		if (!CheckLUA())
		{
			system("CLS");
			EntryLUA(_processID, _processH, _targetAddress, _scrPath.c_str());
		}
		else
		{
			ExecuteLUA();
		}
	}

	void __declspec(dllexport) HookGame(uint64_t ModuleAddress)
	{
		uintptr_t _hookStart = ModuleAddress + 0x14C845;
		uintptr_t _hookEnd = _hookStart + sizeof(_relocatedCode);

		uint8_t _funcBegin[] = {
			PUSHF_1, PUSHF_2,
			PushRax,
			PushRdi,
			PushRbp,
			PushRbx,
			PushR12_1, PushR12_2,
			PushR13_1, PushR13_2,
			PushR14_1, PushR14_2,
			PushR15_1, PushR15_2,
			0x48, 0x83, 0xEC, 0x0E, // sub rsp, 0x0E
			CALL((uintptr_t)&OnFrame),
			0x48, 0x83, 0xC4, 0x0E, // sub rsp, 0x0E
			PopR15_1, PopR15_2,
			PopR14_1, PopR14_2,
			PopR13_1, PopR13_2,
			PopR12_1, PopR12_2,
			PopRbx,
			PopRbp,
			PopRdi,
			PopRax,
			POPF_1, POPF_2,
		};

		uint8_t _funcEnd[] = {
			JUMP_TO(_hookEnd),
		};

		uint64_t _funcSize = sizeof(_funcBegin) + sizeof(_relocatedCode) + sizeof(_funcEnd);

		void* _funcMemory = VirtualAlloc(NULL, _funcSize, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
		uintptr_t _funcAddress = (uintptr_t)_funcMemory;

		uint8_t _patch[] = {
			JUMP_TO(_funcAddress),
        	NOP,
		};

		static_assert(sizeof(_relocatedCode) == sizeof(_patch), "Relocated and patch must be same size!");

		memcpy((void*)_funcAddress, _relocatedCode, sizeof(_relocatedCode));
		memcpy((void*)(_funcAddress + sizeof(_relocatedCode)), _funcBegin, sizeof(_funcBegin));
		memcpy((void*)(_funcAddress + sizeof(_funcBegin) + sizeof(_relocatedCode)), _funcEnd, sizeof(_funcEnd));

		DWORD _originalProt = 0;
		VirtualProtect((void*)_hookStart, sizeof(_patch), PAGE_EXECUTE_READWRITE, &_originalProt);
		memcpy((void*)_hookStart, _patch, sizeof(_patch));
		VirtualProtect((void*)_hookStart, sizeof(_patch), _originalProt, &_originalProt);
	}
}