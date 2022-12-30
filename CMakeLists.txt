cmake_minimum_required(VERSION 3.22)

project(LuaBackendHook C CXX)

set(PROGRAM_VERSION "" CACHE STRING "Program version override")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/subproject/wil/CMakeLists.txt")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

include(FetchContent)

FetchContent_Declare(ztd.text
    GIT_REPOSITORY https://github.com/soasis/text.git
    GIT_SHALLOW    ON
    GIT_TAG        main)

FetchContent_MakeAvailable(ztd.text)

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
    external/subproject/wil/include
    external/include/tomlplusplus/include
    external/include/discord
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_directories(DBGHELP PUBLIC external/lib)
target_link_libraries(DBGHELP PUBLIC lua54 ztd::text discord-rpc.lib)

add_library(DINPUT8 SHARED ${SOURCE})
add_dependencies(DINPUT8 build-info)

target_compile_options(DINPUT8 PRIVATE /W4)
target_compile_definitions(DINPUT8 PRIVATE -DUNICODE -D_UNICODE -DDINPUT8_HOOK)
target_include_directories(DINPUT8 PRIVATE
    external/subproject/lua544
    external/subproject/wil/include
    external/include/tomlplusplus/include
    external/include/discord
    ${CMAKE_CURRENT_BINARY_DIR}
)
target_link_directories(DINPUT8 PUBLIC external/lib)
target_link_libraries(DINPUT8 PUBLIC lua54 ztd::text discord-rpc.lib)

unset(PROGRAM_VERSION CACHE)
