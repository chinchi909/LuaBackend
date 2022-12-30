cmake_minimum_required(VERSION 3.21)

project(LuaBackendHook C CXX)

set(PROGRAM_VERSION "" CACHE STRING "Program version override")

option(WIL_BUILD_TESTS OFF)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(FetchContent)

FetchContent_Declare(ztd.text
    GIT_REPOSITORY https://github.com/soasis/text.git
    GIT_SHALLOW    ON
    GIT_TAG        main)

FetchContent_Declare(wil
    GIT_REPOSITORY https://github.com/microsoft/wil.git
    GIT_SHALLOW    ON
    GIT_TAG        master)

FetchContent_Declare(tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_SHALLOW    ON
    GIT_TAG        master)

FetchContent_MakeAvailable(ztd.text wil tomlplusplus)

include_directories(external/include)

add_subdirectory(external/subproject/lua544)

if(NOT MSVC)
    message(FATAL_ERROR "LuaBackendHook must be built using MSVC.")
endif()

set(HEADERS
    src/config.h
    src/game_info.h
    src/lua_exec.h
    src/wil_extra.h
    src/header_text.h
    src/ConsoleLib.h
    src/DCInstance.h
    src/LuaBackend.h
    src/MemoryLib.h
)

set(SOURCE
    src/main_dll.cpp
    src/config.cpp
    src/lua_exec.cpp
    src/header_text.cpp
    src/ConsoleLib.cpp
    src/DCInstance.cpp
    src/LuaBackend.cpp
)

set(ALL_FILES ${HEADERS} ${SOURCE})

add_custom_target(clang-format
    COMMAND clang-format -i -style=file ${ALL_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(build-info
    COMMAND ${CMAKE_COMMAND}
        -DPROGRAM_VERSION:STRING=${PROGRAM_VERSION}
        -DINSTALL_DIR:STRING=${CMAKE_CURRENT_BINARY_DIR}
        -P CMake/BuildInfo.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
)

add_library(DBGHELP SHARED ${SOURCE})
add_dependencies(DBGHELP build-info)

target_compile_options(DBGHELP PRIVATE /W4)
target_compile_definitions(DBGHELP PRIVATE -DUNICODE -D_UNICODE)
target_include_directories(DBGHELP PRIVATE
    external/subproject/lua544
    external/include/discord
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_directories(DBGHELP PUBLIC external/lib)
target_link_libraries(DBGHELP PUBLIC
    ztd::text
    WIL
    tomlplusplus::tomlplusplus
    lua54
    discord-rpc.lib
)

unset(PROGRAM_VERSION CACHE)
