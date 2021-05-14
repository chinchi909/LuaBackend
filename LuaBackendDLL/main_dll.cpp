#include <chrono>
#include <iostream>
#include <filesystem>

#include <windows.h>

#include <MemoryLib.h>
#include <LuaBackend.h>
#include <mIni/ini.h>

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

			if (_msTime > _backend->frameLimit)
			{
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
			}

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