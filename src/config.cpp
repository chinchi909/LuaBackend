#include "config.h"

#include <toml++/toml.h>

#include <array>
#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <utility>
#include <vector>

namespace fs = std::filesystem;

constexpr std::array<const char*, 4> keys{"kh1", "kh2", "bbs", "recom"};

struct Entry {
    std::u8string scripts;
    bool documents_relative;
    std::uintptr_t base;
    std::uintptr_t thread_struct;
    std::u8string exe;
};

Config Config::load(const fs::path& path) {
    Config config;
    auto data = toml::parse_file(path.u8string());

    for (const auto& k : keys) {
        if (!data.contains(k)) continue;

        const auto& entry = data[k];

        const auto scripts = entry["scripts"].as_array();
        const auto base = entry["base"].value<std::uintptr_t>();
        const auto threadStruct =
            entry["thread_struct"].value<std::uintptr_t>();
        const auto exe = entry["exe"].value<std::u8string>();

        if (!scripts) {
            throw std::runtime_error{std::string{k} +
                                     ": scripts failed to parse"};
        }
        if (!base) {
            throw std::runtime_error{std::string{k} + ": base failed to parse"};
        }
        if (!threadStruct) {
            throw std::runtime_error{std::string{k} +
                                     ": threadStruct failed to parse"};
        }
        if (!exe) {
            throw std::runtime_error{std::string{k} + ": exe failed to parse"};
        }

        std::vector<ScriptPath> paths;
        for (std::size_t i = 0; i < scripts->size(); i++) {
            const auto& v = *scripts->get_as<toml::table>(i);
            const auto str = v["path"].value<std::u8string>();
            const auto relative = v["relative"].value<bool>();

            if (!str) {
                throw std::runtime_error{std::string{k} + ": at index " +
                                         std::to_string(i) +
                                         ": script entry path failed to parse"};
            }
            if (!relative) {
                throw std::runtime_error{
                    std::string{k} + ": at index " + std::to_string(i) +
                    ": script entry relative failed to parse"};
            }

            paths.push_back({std::move(*str), *relative});
        }

        GameInfo info{
            *threadStruct,
            *base,
            paths,
        };

        config.infos.insert({std::move(*exe), info});
    }

    return config;
}

std::optional<std::reference_wrapper<const GameInfo>> Config::gameInfo(
    const std::u8string& exe) const {
    if (auto info = infos.find(exe); info != infos.end()) {
        return std::cref(info->second);
    } else {
        return {};
    }
}
