cmake_minimum_required(VERSION 3.13)

project(LuaBackendHook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(DBGHELP SHARED
    src/main_dll.cpp
    src/DCInstance.cpp
    src/GitRequest.cpp
    src/LuaBackend.cpp
)

target_include_directories(DBGHELP PUBLIC
    include
    include/lua
    include/discord
)

target_link_directories(DBGHELP PUBLIC
    lib/lua
    lib/discord
)

target_compile_definitions(DBGHELP PRIVATE -DUNICODE -D_UNICODE)

target_link_libraries(DBGHELP PUBLIC lua54.lib PUBLIC discord-rpc.lib)
