cmake_minimum_required(VERSION 3.23)

project(LuaBackendHook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(external/include)

add_subdirectory(external/subproject/lua544)

if(NOT MSVC)
    message(FATAL_ERROR "LuaBackendHook must be built using MSVC.")
endif()

add_library(DBGHELP SHARED
    src/main_dll.cpp
    src/DCInstance.cpp
    src/LuaBackend.cpp
    src/config.cpp
)

target_compile_definitions(DBGHELP PRIVATE -DUNICODE -D_UNICODE)

target_include_directories(DBGHELP PRIVATE external/subproject/lua544 external/include/discord)
target_link_directories(DBGHELP PUBLIC external/lib)
target_link_libraries(DBGHELP PUBLIC lua54 discord-rpc.lib)
