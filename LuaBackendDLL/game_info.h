#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

struct HookInfo {
    std::uintptr_t start;
    std::size_t size;
};

struct GameInfo {
    HookInfo hookInfo;
    std::uintptr_t baseAddress;
    std::string_view scriptsPath;
};
