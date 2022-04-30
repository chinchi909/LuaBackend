#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

struct ScriptPath {
    std::string str;
    bool relative;
};

struct GameInfo {
    std::uintptr_t pointerStructOffset;
    std::uintptr_t baseAddress;
    std::vector<ScriptPath> scriptPaths;
};
