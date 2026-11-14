#include "ui/button_input.h"

namespace ui {

namespace {

NavInput shortPressInput(hal::ButtonId id) {
  switch (id) {
    case hal::ButtonId::kLeft:
      return NavInput::kLeftShort;
    case hal::ButtonId::kCenter:
      return NavInput::kCenterShort;
    case hal::ButtonId::kRight:
    default:
      return NavInput::kRightShort;
  }
}

}  // namespace

uint8_t ButtonInput::poll(const hal::Buttons& buttons, uint32_t nowMs, NavInput* out) {
  uint8_t count = 0;

  for (uint8_t i = 0; i < hal::kButtonCount; ++i) {
    const hal::ButtonId id = static_cast<hal::ButtonId>(i);
    const GestureEvent event = gestures_[i].update(buttons.isPressed(id), nowMs);

    if (event == GestureEvent::kShortPress) {
      out[count++] = shortPressInput(id);
    } else if (event == GestureEvent::kLongPress && id == hal::ButtonId::kCenter) {
      out[count++] = NavInput::kCenterLong;
    }
  }

  return count;
}

}  // namespace ui