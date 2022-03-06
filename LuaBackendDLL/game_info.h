#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

struct GameInfo {
    std::uintptr_t pointerStructOffset;
    std::uintptr_t baseAddress;
    std::uintptr_t hookAddress;
    std::string_view scriptsPath;
};
