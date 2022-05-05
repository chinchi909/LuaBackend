#include "lua_exec.h"

#include <utility>

#include "ConsoleLib.h"
#include "LuaBackend.h"

namespace fs = std::filesystem;

static bool _funcOneState = true;
static bool _funcTwoState = true;
static bool _funcThreeState = true;

static std::vector<fs::path> _scriptPaths;

static bool _showConsole = false;
static bool _requestedReset = false;

LuaBackend* _backend = nullptr;

std::chrono::high_resolution_clock::time_point _sClock;
std::chrono::high_resolution_clock::time_point _msClock;

void EnterWait() {
    std::string _output;
    std::getline(std::cin, _output);
    return;
}

void ResetLUA() {
    std::printf("\n");
    ConsoleLib::MessageOutput("Reloading...\n\n", 0);
    _backend = new LuaBackend(_scriptPaths,
                              MemoryLib::ExecAddress + MemoryLib::BaseAddress);

    if (_backend->loadedScripts.size() == 0)
        ConsoleLib::MessageOutput("No scripts found! Reload halted!\n\n", 3);

    ConsoleLib::MessageOutput("Executing initialization event handlers...\n\n",
                              0);

    for (auto _script : _backend->loadedScripts)
        if (_script->initFunction) {
            auto _result = _script->initFunction();

            if (!_result.valid()) {
                sol::error _err = _result;
                ConsoleLib::MessageOutput(_err.what(), 3);
                std::printf("\n\n");
            }
        }

    ConsoleLib::MessageOutput("Reload complete!\n\n", 1);

    _msClock = std::chrono::high_resolution_clock::now();
    _sClock = std::chrono::high_resolution_clock::now();

    _requestedReset = false;
}

int EntryLUA(int ProcessID, HANDLE ProcessH, std::uint64_t TargetAddress,
             std::vector<fs::path> ScriptPaths) {
    ShowWindow(GetConsoleWindow(), SW_HIDE);

    std::cout << "======================================"
              << "\n";
    std::cout << "======= LuaBackendHook | v1.5.1 ======"
              << "\n";
    std::cout << "====== Copyright 2021 - TopazTK ======"
              << "\n";
    std::cout << "======================================"
              << "\n";
    std::cout << "=== Compatible with LuaEngine v5.0 ==="
              << "\n";
    std::cout << "========== Embedded Version =========="
              << "\n";
    std::cout << "======================================"
              << "\n\n";

    ConsoleLib::MessageOutput("Initializing LuaEngine v5.0...\n\n", 0);
    _scriptPaths = std::move(ScriptPaths);

    MemoryLib::ExternProcess(ProcessID, ProcessH, TargetAddress);

    _backend =
        new LuaBackend(ScriptPaths, MemoryLib::ExecAddress + TargetAddress);
    _backend->frameLimit = 16;

    if (_backend->loadedScripts.size() == 0) {
        ConsoleLib::MessageOutput(
            "No scripts were found! Initialization halted!\n\n", 3);
        return -1;
    }

    ConsoleLib::MessageOutput("Executing initialization event handlers...\n\n",
                              0);

    for (auto _script : _backend->loadedScripts)
        if (_script->initFunction) {
            auto _result = _script->initFunction();

            if (!_result.valid()) {
                sol::error _err = _result;
                ConsoleLib::MessageOutput(_err.what(), 3);
                std::printf("\n\n");
            }
        }

    ConsoleLib::MessageOutput("Initialization complete!\n", 1);
    ConsoleLib::MessageOutput(
        "Press 'F1' to reload all scripts, press 'F2' to toggle the console, "
        "press 'F3' to set "
        "execution frequency.\n\n",
        0);

    _msClock = std::chrono::high_resolution_clock::now();
    _sClock = std::chrono::high_resolution_clock::now();

    return 0;
}

void ExecuteLUA() {
    if (_requestedReset == false) {
        auto _currTime = std::chrono::high_resolution_clock::now();
        auto _sTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                          _currTime - _sClock)
                          .count();

        if (GetKeyState(VK_F3) & 0x8000 && _funcThreeState) {
            switch (_backend->frameLimit) {
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
            _sClock = std::chrono::high_resolution_clock::now();
        }
        if (GetKeyState(VK_F2) & 0x8000 && _funcTwoState) {
            if (_showConsole) {
                ShowWindow(GetConsoleWindow(), SW_HIDE);
                _showConsole = false;
            }

            else {
                ShowWindow(GetConsoleWindow(), SW_RESTORE);
                _showConsole = true;
            }

            _sTime = 0;
            _funcTwoState = false;
            _sClock = std::chrono::high_resolution_clock::now();
        }
        if (GetKeyState(VK_F1) & 0x8000 && _funcOneState) {
            _requestedReset = true;

            _sTime = 0;
            _funcOneState = false;
            _sClock = std::chrono::high_resolution_clock::now();
        }

        for (int i = 0; i < _backend->loadedScripts.size(); i++) {
            auto _script = _backend->loadedScripts[i];

            if (_script->frameFunction) {
                auto _result = _script->frameFunction();

                if (!_result.valid()) {
                    sol::error _err = _result;
                    ConsoleLib::MessageOutput(_err.what(), 3);
                    std::printf("\n\n");

                    _backend->loadedScripts.erase(
                        _backend->loadedScripts.begin() + i);
                }
            }
        }

        _msClock = std::chrono::high_resolution_clock::now();

        if (_sTime > 250) {
            _funcOneState = true;
            _funcTwoState = true;
            _funcThreeState = true;
            _sClock = std::chrono::high_resolution_clock::now();
        }
    }

    else
        ResetLUA();
}

bool CheckLUA() {
    auto _int = MemoryLib::ReadInt(0);

    if (_int == 0) return false;

    return true;
}

int VersionLUA() { return 128; }
