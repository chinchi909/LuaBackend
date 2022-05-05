#ifndef LUABACKEND
#define LUABACKEND

#include <filesystem>
#include <iostream>
#include <sol/sol.hpp>

#include "ConsoleLib.h"
#include "DCInstance.h"
#include "MemoryLib.h"

using LuaState = sol::state;
using LuaFunction = sol::safe_function;

class LuaBackend {
   public:
    struct LuaScript {
        LuaState luaState;
        LuaFunction initFunction;
        LuaFunction frameFunction;
    };

    std::vector<LuaScript*> loadedScripts;
    int frameLimit;

    static int ExceptionHandle(
        lua_State* luaState,
        sol::optional<const std::exception&> thrownException,
        sol::string_view description) {
        (void)description;

        const std::exception _ex = *thrownException;
        ConsoleLib::MessageOutput(_ex.what() + '\n', 3);

        return sol::stack::push(luaState, _ex.what());
    }

    void SetFunctions(LuaState*);
    void LoadScripts(std::vector<std::filesystem::path>, std::uint64_t);
    LuaBackend(std::vector<std::filesystem::path>, std::uint64_t);
};

#endif
