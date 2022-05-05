#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "game_info.h"

class Config {
   private:
    std::unordered_map<std::string, GameInfo> infos;

   public:
    static Config load(std::filesystem::path path);
    std::optional<std::reference_wrapper<const GameInfo>> gameInfo(
        const std::string& exe) const;
};
