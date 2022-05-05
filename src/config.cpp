#include "config.h"

#include <array>
#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <toml11/toml.hpp>
#include <utility>

constexpr std::array<const char*, 4> keys{"kh1", "kh2", "bbs", "recom"};

struct Entry {
    std::string scripts;
    bool documents_relative;
    std::uintptr_t base;
    std::uintptr_t thread_struct;
    std::string exe;
};

static bool stringToInt(const std::string& str, unsigned long& ret, int base) {
    auto [ptr, ec] =
        std::from_chars(str.data(), str.data() + str.size(), ret, base);
    return ptr[0] == '\0' && ec == std::errc();
}

Config Config::load(std::string_view path) {
    Config config;
    auto data = toml::parse(path);

    for (const auto& k : keys) {
        if (!data.contains(k)) continue;

        const auto entry = toml::find(data, k);

        const auto scripts = toml::find(entry, "scripts");
        const auto base = toml::find<std::string>(entry, "base");
        const auto threadStruct =
            toml::find<std::string>(entry, "thread_struct");
        const auto exe = toml::find<std::string>(entry, "exe");

        std::vector<ScriptPath> paths;
        for (const auto& v : scripts.as_array()) {
            const auto str = toml::find<std::string>(v, "path");
            const auto relative = toml::find<bool>(v, "relative");
            paths.push_back({std::move(str), relative});
        }

        unsigned long threadStructVal;
        unsigned long baseVal;

        if (!stringToInt(threadStruct, threadStructVal, 16)) {
            throw std::runtime_error{
                "threadStruct failed to parse with value \"" + threadStruct +
                "\""};
        }

        if (!stringToInt(base, baseVal, 16)) {
            throw std::runtime_error{
                "threadStruct failed to parse with value \"" + base + "\""};
        }

        GameInfo info{
            threadStructVal,
            baseVal,
            paths,
        };

        config.infos.insert({exe, info});
    }

    return config;
}

std::optional<std::reference_wrapper<const GameInfo>> Config::gameInfo(
    const std::string& exe) const {
    if (auto info = infos.find(exe); info != infos.end()) {
        return std::cref(info->second);
    } else {
        return {};
    }
}
