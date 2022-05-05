#include "LuaBackend.h"

#include <crcpp/CRC.h>

#include <utility>

namespace fs = std::filesystem;

LuaBackend::LuaBackend(std::vector<fs::path> ScriptPaths,
                       std::uint64_t BaseInput) {
    frameLimit = 16;
    LoadScripts(ScriptPaths, BaseInput);
}

int LuaBackend::ExceptionHandle(
    lua_State* luaState, sol::optional<const std::exception&> thrownException,
    sol::string_view description) {
    (void)description;

    const std::exception _ex = *thrownException;
    ConsoleLib::MessageOutput(_ex.what() + '\n', 3);

    return sol::stack::push(luaState, _ex.what());
}

void LuaBackend::LoadScripts(std::vector<fs::path> ScriptPaths,
                             std::uint64_t BaseInput) {
    loadedScripts.clear();

    for (auto scriptsDir : ScriptPaths) {
        if (!fs::is_directory(scriptsDir)) {
            continue;
        }

        for (auto _path : fs::directory_iterator(scriptsDir)) {
            auto _script = std::make_unique<LuaScript>();

            // clang-format off
            _script->luaState.open_libraries(
                sol::lib::base,
                sol::lib::package,
                sol::lib::coroutine,
                sol::lib::string,
                sol::lib::os,
                sol::lib::math,
                sol::lib::table,
                sol::lib::io,
                sol::lib::bit32,
                sol::lib::utf8
            );
            // clang-format on

            _script->luaState.set_exception_handler(&ExceptionHandle);

            SetFunctions(&_script->luaState);

            fs::path _luaPath = scriptsDir / L"io_packages" / L"?.lua";
            fs::path _dllPath = scriptsDir / L"io_packages" / L"?.dll";

            _script->luaState["package"]["path"] = _luaPath.string();
            _script->luaState["package"]["cpath"] = _dllPath.string();

            fs::path _loadPath = scriptsDir / L"io_load";

            _script->luaState["LOAD_PATH"] = _loadPath.string();
            _script->luaState["SCRIPT_PATH"] = scriptsDir.string();
            _script->luaState["CHEATS_PATH"] = "NOT_AVAILABLE";

            fs::path _pathFull = MemoryLib::PName;
            auto _pathExe = _pathFull.filename().string();

            _script->luaState["ENGINE_VERSION"] = 5;
            _script->luaState["ENGINE_TYPE"] = "BACKEND";
            _script->luaState["GAME_ID"] = CRC::Calculate(
                _pathExe.c_str(), _pathExe.length(), CRC::CRC_32());
            _script->luaState["BASE_ADDR"] = BaseInput;

            const auto _filePath = _path.path();
            const auto _filePathStr = _filePath.string();

            if (_filePath.extension() == ".lua") {
                _script->luaState["LUA_NAME"] = _filePath.filename().string();

                ConsoleLib::MessageOutput(
                    "Found script: \"" + _filePathStr + "\" Initializing...\n",
                    0);

                auto _result = _script->luaState.script_file(
                    _filePathStr, &sol::script_pass_on_error);

                _script->initFunction = _script->luaState["_OnInit"];
                _script->frameFunction = _script->luaState["_OnFrame"];

                if (_result.valid()) {
                    if (!_script->initFunction && !_script->frameFunction) {
                        ConsoleLib::MessageOutput(
                            "No event handlers exist or all of them have "
                            "errors.\n",
                            3);
                        ConsoleLib::MessageOutput(
                            "Initialization of this script cannot "
                            "continue...\n",
                            3);
                        return;
                    }

                    if (!_script->initFunction)
                        ConsoleLib::MessageOutput(
                            "The event handler for initialization either has "
                            "errors or "
                            "does not exist.\n",
                            2);

                    if (!_script->frameFunction)
                        ConsoleLib::MessageOutput(
                            "The event handler for framedraw either has errors "
                            "or does not "
                            "exist.\n",
                            2);

                    ConsoleLib::MessageOutput(
                        "Initialization of this script was successful!\n\n", 1);

                    loadedScripts.push_back(std::move(_script));
                } else {
                    sol::error err = _result;
                    ConsoleLib::MessageOutput(err.what() + '\n', 3);
                    ConsoleLib::MessageOutput(
                        "Initialization of this script was aborted.\n", 3);
                }
            }
        }
    }
}

void LuaBackend::SetFunctions(LuaState* _state) {
    _state->set_function("ReadByte", MemoryLib::ReadByte);
    _state->set_function("ReadShort", MemoryLib::ReadShort);
    _state->set_function("ReadInt", MemoryLib::ReadInt);
    _state->set_function("ReadLong", MemoryLib::ReadLong);
    _state->set_function("ReadFloat", MemoryLib::ReadFloat);
    _state->set_function("ReadBoolean", MemoryLib::ReadBool);
    _state->set_function("ReadArray", MemoryLib::ReadBytes);
    _state->set_function("ReadString", MemoryLib::ReadString);

    _state->set_function("WriteByte", MemoryLib::WriteByte);
    _state->set_function("WriteShort", MemoryLib::WriteShort);
    _state->set_function("WriteInt", MemoryLib::WriteInt);
    _state->set_function("WriteLong", MemoryLib::WriteLong);
    _state->set_function("WriteFloat", MemoryLib::WriteFloat);
    _state->set_function("WriteBoolean", MemoryLib::WriteBool);
    _state->set_function("WriteArray", MemoryLib::WriteBytes);
    _state->set_function("WriteString", MemoryLib::WriteString);
    _state->set_function("WriteExec", MemoryLib::WriteExec);
    _state->set_function("GetPointer", MemoryLib::GetPointer);

    _state->set_function("ReadByteA", MemoryLib::ReadByteAbsolute);
    _state->set_function("ReadShortA", MemoryLib::ReadShortAbsolute);
    _state->set_function("ReadIntA", MemoryLib::ReadIntAbsolute);
    _state->set_function("ReadLongA", MemoryLib::ReadLongAbsolute);
    _state->set_function("ReadFloatA", MemoryLib::ReadFloatAbsolute);
    _state->set_function("ReadBooleanA", MemoryLib::ReadBoolAbsolute);
    _state->set_function("ReadArrayA", MemoryLib::ReadBytesAbsolute);
    _state->set_function("ReadStringA", MemoryLib::ReadStringAbsolute);

    _state->set_function("WriteByteA", MemoryLib::WriteByteAbsolute);
    _state->set_function("WriteShortA", MemoryLib::WriteShortAbsolute);
    _state->set_function("WriteIntA", MemoryLib::WriteIntAbsolute);
    _state->set_function("WriteLongA", MemoryLib::WriteLongAbsolute);
    _state->set_function("WriteFloatA", MemoryLib::WriteFloatAbsolute);
    _state->set_function("WriteBooleanA", MemoryLib::WriteBoolAbsolute);
    _state->set_function("WriteArrayA", MemoryLib::WriteBytesAbsolute);
    _state->set_function("WriteStringA", MemoryLib::WriteStringAbsolute);
    _state->set_function("GetPointerA", MemoryLib::GetPointerAbsolute);

    _state->set_function("InitializeRPC", DCInstance::InitializeRPC);
    _state->set_function("UpdateDetails", DCInstance::UpdateDetails);
    _state->set_function("UpdateState", DCInstance::UpdateState);
    _state->set_function("UpdateLImage", DCInstance::UpdateLImage);
    _state->set_function("UpdateSImage", DCInstance::UpdateSImage);

    _state->set_function(
        "ULShift32",
        [](std::uint32_t base, std::uint32_t shift) { return base << shift; });

    _state->set_function(
        "ConsolePrint",
        sol::overload(
            [_state](sol::object Text) {
                HANDLE _hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

                SetConsoleTextAttribute(_hConsole, 14);
                std::cout
                    << "[" + _state->globals()["LUA_NAME"].get<std::string>() +
                           "] ";

                SetConsoleTextAttribute(_hConsole, 7);
                std::cout << Text.as<std::string>() << '\n';
            },

            [_state](sol::object Text, int MessageType) {
                HANDLE _hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

                SetConsoleTextAttribute(_hConsole, 14);
                std::cout
                    << "[" + _state->globals()["LUA_NAME"].get<std::string>() +
                           "] ";

                switch (MessageType) {
                    case 0:
                        SetConsoleTextAttribute(_hConsole, 11);
                        std::cout << "MESSAGE: ";
                        break;
                    case 1:
                        SetConsoleTextAttribute(_hConsole, 10);
                        std::cout << "SUCCESS: ";
                        break;
                    case 2:
                        SetConsoleTextAttribute(_hConsole, 14);
                        std::cout << "WARNING: ";
                        break;
                    case 3:
                        SetConsoleTextAttribute(_hConsole, 12);
                        std::cout << "ERROR: ";
                        break;
                }

                SetConsoleTextAttribute(_hConsole, 7);
                std::cout << Text.as<std::string>() << '\n';
            }));

    _state->set_function("GetHertz", [this]() {
        switch (frameLimit) {
            default:
                return 60;
            case 8:
                return 120;
            case 4:
                return 240;
        }
    });

    _state->set_function("SetHertz", [this](int Input) {
        switch (Input) {
            default:
                frameLimit = 16;
                break;
            case 120:
                frameLimit = 8;
                break;
            case 240:
                frameLimit = 4;
                break;
        }
    });
}
