cmake_minimum_required(VERSION 3.13)

project(LuaBackendHook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(DBGHELP SHARED
    src/main_dll.cpp
    src/DCInstance.cpp
    src/LuaBackend.cpp
    src/config.cpp
)

target_include_directories(DBGHELP PUBLIC
    include
    include/lua
    include/discord
)

target_link_directories(DBGHELP PUBLIC lib)

target_compile_definitions(DBGHELP PRIVATE -DUNICODE -D_UNICODE)

target_link_libraries(DBGHELP PUBLIC lua.lib PUBLIC discord-rpc.lib)
