#include "kh2.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <ztd/text.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "wil_extra.h"

namespace fs = std::filesystem;

using ProcessHealFn = int(void* thisx, int amount, int param_3, char param_4);

namespace kh2 {
static std::byte* get_mod() {
  return std::bit_cast<std::byte*>(GetModuleHandleW(nullptr));
}

bool is_kh2_global() {
  std::wstring module_path_str;
  wil::GetModuleFileNameW(nullptr, module_path_str);

  fs::path module_path = fs::path{module_path_str};
  fs::path module_dir = module_path.parent_path();
  std::wstring module_name_w = module_path.filename();

  std::u8string module_name =
      ztd::text::transcode(module_name_w, ztd::text::wide_utf16,
                           ztd::text::utf8, ztd::text::replacement_handler);

  if (module_name != u8"KINGDOM HEARTS II FINAL MIX.exe") {
    return false;
  }

  std::byte* mod = get_mod();
  std::byte game_version = *(mod + 0x17D);

  return game_version == std::byte{0x68};
}

bool die(bool mickey_counts) {
  std::byte* mod = get_mod();
  std::byte* player_ptr = *std::bit_cast<std::byte**>(mod + 0xABA7E8);
  if (player_ptr == nullptr)
    return false;

  // Wait for player to have control and not be in a menu before killing.
  int control = *std::bit_cast<int*>(mod + 0x2A148E8);
  int pause_flag = *std::bit_cast<int*>(mod + 0xAB9054);
  if (control != 0 || (pause_flag & ~2) != 0)
    return false;

  int* player_hp = *std::bit_cast<int**>(player_ptr + 0x5C0);

  int player_form = *std::bit_cast<int*>(player_ptr + 0xDE0);
  bool is_mickey = player_form == 0xB;

  // Prevent becoming Mickey from counting as a death.
  if (*player_hp == 0)
    return false;

  if (is_mickey) {
    *player_hp = 1;
    return mickey_counts;
  }

  auto heal_fn = std::bit_cast<ProcessHealFn*>(mod + 0x3D0720);
  heal_fn(player_ptr, -*player_hp, 0, 0);

  return true;
}
} // namespace kh2
