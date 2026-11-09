#pragma once

#include <stdint.h>

namespace ui {

enum class GestureEvent : uint8_t { kNone = 0, kShortPress, kLongPress };

struct GestureConfig {
  uint32_t debounceMs = 30;
  uint32_t longPressMs = 800;
};

// 버튼 하나의 눌림 상태를 짧게/길게 누름 이벤트로 바꿉니다.
// 버튼마다 인스턴스를 하나씩 만들고, 루프마다 update()를 호출합니다.
class ButtonGesture {
 public:
  explicit ButtonGesture(GestureConfig config = GestureConfig());

  GestureEvent update(bool rawPressed, uint32_t nowMs);

 private:
  GestureConfig config_;
  bool stable_ = false;
  bool lastRaw_ = false;
  uint32_t rawChangedAt_ = 0;
  uint32_t pressedAt_ = 0;
  bool longFired_ = false;
};

}  // namespace ui