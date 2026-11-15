#include "ui/nav_input_mapper.h"

namespace ui {

bool NavInputMapper::update(const bool pressed[hal::kButtonCount], uint32_t nowMs, NavInput& out) {
  bool found = false;

  // 이벤트를 놓치지 않도록 버튼 3개를 항상 모두 갱신합니다.
  for (uint8_t i = 0; i < hal::kButtonCount; ++i) {
    const GestureEvent event = gestures_[i].update(pressed[i], nowMs);
    if (found || event == GestureEvent::kNone) {
      continue;
    }

    const bool isCenter = i == static_cast<uint8_t>(hal::ButtonId::kCenter);
    if (event == GestureEvent::kLongPress) {
      if (isCenter) {
        out = NavInput::kCenterLong;
        found = true;
      }
      continue;  // 왼쪽/오른쪽 길게 누름은 아직 사용하지 않습니다.
    }

    if (i == static_cast<uint8_t>(hal::ButtonId::kLeft)) {
      out = NavInput::kLeftShort;
    } else if (isCenter) {
      out = NavInput::kCenterShort;
    } else {
      out = NavInput::kRightShort;
    }
    found = true;
  }

  return found;
}

}  // namespace ui