cmake_minimum_required(VERSION 3.13)

project(LuaBackendHook)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# LuaBackendLIB

add_library(LuaBackendLIB STATIC
    LuaBackendLIB/source/DCInstance.cpp
    LuaBackendLIB/source/GitRequest.cpp
    LuaBackendLIB/source/LuaBackend.cpp
)

target_include_directories(LuaBackendLIB PUBLIC
    LuaBackendLIB/include
    LuaBackendLIB/include/lua
    LuaBackendLIB/include/discord
)

target_link_directories(LuaBackendLIB PUBLIC
    LuaBackendLIB/libraries/lua
    LuaBackendLIB/libraries/discord
)

target_link_libraries(LuaBackendLIB PUBLIC lua54.lib PUBLIC discord-rpc.lib)

target_compile_definitions(LuaBackendLIB PRIVATE -DUNICODE -D_UNICODE)

# DINPUT8

add_library(DINPUT8 SHARED
    LuaBackendDLL/main_dll.cpp

    LuaBackendDLL/imgui.cpp
    LuaBackendDLL/imgui_demo.cpp
    LuaBackendDLL/imgui_draw.cpp
    LuaBackendDLL/imgui_tables.cpp
    LuaBackendDLL/imgui_widgets.cpp

    LuaBackendDLL/backends/imgui_impl_dx12.cpp
    LuaBackendDLL/backends/imgui_impl_win32.cpp
)

target_link_directories(DINPUT8 PRIVATE LuaBackendLIB)

target_compile_definitions(DINPUT8 PRIVATE -DUNICODE -D_UNICODE)

target_link_libraries(DINPUT8 LuaBackendLIB)
