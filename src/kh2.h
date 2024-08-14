#pragma once

namespace kh2 {
bool is_kh2_global();

/// Kills the player. Returns whether or not they were successfully killed.
/// `mickey_counts` Indicates whether or not Mickey should count as a death.
/// If false, both Mickey and Sora will die.
bool die(bool mickey_counts);
} // namespace kh2
