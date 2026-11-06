#include "core/mission_generator.h"

namespace maeumgochi {

std::optional<uint8_t> calculateMissionDifficulty(int mood, int energy) {
  constexpr int kMinimumCheckInValue = 1;
  constexpr int kMaximumCheckInValue = 5;

  if (mood < kMinimumCheckInValue || mood > kMaximumCheckInValue ||
      energy < kMinimumCheckInValue || energy > kMaximumCheckInValue) {
    return std::nullopt;
  }

  if (mood == kMinimumCheckInValue || energy == kMinimumCheckInValue) {
    return static_cast<uint8_t>(kMinimumCheckInValue);
  }

  return static_cast<uint8_t>((mood + energy + 1) / 2);
}

}  // namespace maeumgochi
