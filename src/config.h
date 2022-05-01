#pragma once

#include <string>
#include <string_view>
#include <optional>
#include <unordered_map>
#include <functional>

#include "game_info.h"

class Config {
private:
    std::unordered_map<std::string, GameInfo> infos;
    bool _preventMapCrash = false;

public:
    static Config load(std::string_view path);
    std::optional<std::reference_wrapper<const GameInfo>> gameInfo(const std::string& exe) const;
    bool preventMapCrash() const { return _preventMapCrash; }
};
