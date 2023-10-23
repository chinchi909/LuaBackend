cmake_minimum_required(VERSION 3.7...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(Lua LANGUAGES C)

set(LUAOBJ
    lapi.c
    lcode.c
    lctype.c
    ldebug.c
    ldo.c
    ldump.c
    lfunc.c
    lgc.c
    llex.c
    lmem.c
    lobject.c
    lopcodes.c
    lparser.c
    lstate.c
    lstring.c
    ltable.c
    ltm.c
    lundump.c
    lvm.c
    lzio.c
    lauxlib.c
    lbaselib.c
    lcorolib.c
    ldblib.c
    liolib.c
    lmathlib.c
    loadlib.c
    loslib.c
    lstrlib.c
    ltablib.c
    lutf8lib.c
    linit.c
)

add_library(lualib STATIC ${LUAOBJ})
target_compile_features(lualib PRIVATE c_std_99)
set_target_properties(lualib PROPERTIES
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)
target_compile_definitions(lualib PRIVATE LUA_BUILD_AS_DLL)

add_library(lua54 SHARED ${LUAOBJ})
target_compile_features(lua54 PRIVATE c_std_99)
set_target_properties(lua54 PROPERTIES
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)
target_compile_definitions(lua54 PRIVATE LUA_BUILD_AS_DLL)

add_executable(lua lua.c)
target_compile_features(lua PRIVATE c_std_99)
set_target_properties(lua PROPERTIES
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
)
target_link_libraries(lua PRIVATE lua54)
