#include "header_text.h"

#include <version.h>

#include <algorithm>
#include <array>
#include <cstddef>

template <std::size_t... Len>
[[nodiscard]] constexpr static auto generateBlockText(const char (&... lines)[Len]) {
    constexpr std::string_view separator = " ";
    constexpr std::string_view padding = "=";
    constexpr std::size_t edge_pad_amount = 3;

    constexpr std::size_t max_line_length = std::max({Len...}) - 1;
    constexpr std::size_t num_lines = sizeof...(Len);

    constexpr std::size_t formatted_line_size =
        max_line_length + (2 * separator.size()) + (edge_pad_amount * 2 * padding.size());

    constexpr std::size_t text_size = (formatted_line_size + 1) * num_lines + 1;
    std::array<char, text_size> text;
    text[text.size() - 1] = '\0';

    char* dst = text.data();
    for (const char* src_raw : {lines...}) {
        std::string_view src = src_raw;

        std::size_t sep_size = src.empty() ? 0 : separator.size();
        std::size_t total_pad = formatted_line_size - src.size() - (2 * sep_size);
        std::size_t right_pad = total_pad / 2;
        std::size_t left_pad = total_pad - right_pad;

        const auto copy = [](auto& dst, const auto& src) {
            std::copy(src.begin(), src.end(), dst);
            dst += src.size();
        };

        const auto copy_n_times = [&copy](auto& dst, const auto& src, std::size_t n) {
            for (std::size_t i = 0; i < n; i++) copy(dst, src);
        };

        copy_n_times(dst, padding, left_pad);
        copy_n_times(dst, separator, sep_size);
        copy(dst, src);
        copy_n_times(dst, separator, sep_size);
        copy_n_times(dst, padding, right_pad);
        *dst++ = '\n';
    }

    return text;
}

std::string_view getHeaderText() {
    // clang-format off
    constexpr static auto headerText = generateBlockText(
        "",
        "LuaBackendHook " HOOK_VERSION,
        "Copyright 2022 - TopazTK | Sirius902",
        "",
        "Compatible with LuaEngine v5.0",
        "Embedded Version",
        ""
    );
    // clang-format on
    constexpr static auto headerTextView = std::string_view{headerText.begin(), headerText.end()};
    return headerTextView;
}
