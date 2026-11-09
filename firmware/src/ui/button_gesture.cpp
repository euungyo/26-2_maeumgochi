#include "ui/button_gesture.h"

namespace ui {

ButtonGesture::ButtonGesture(GestureConfig config) : config_(config) {}

GestureEvent ButtonGesture::update(bool rawPressed, uint32_t nowMs) {
  if (rawPressed != lastRaw_) {
    lastRaw_ = rawPressed;
    rawChangedAt_ = nowMs;
  }

  // 입력이 debounceMs 동안 유지되면 상태 변화로 인정합니다.
  if (rawPressed != stable_ && nowMs - rawChangedAt_ >= config_.debounceMs) {
    stable_ = rawPressed;
    if (stable_) {
      pressedAt_ = nowMs;
      longFired_ = false;
      return GestureEvent::kNone;
    }
    return longFired_ ? GestureEvent::kNone : GestureEvent::kShortPress;
  }

  if (stable_ && !longFired_ && nowMs - pressedAt_ >= config_.longPressMs) {
    longFired_ = true;
    return GestureEvent::kLongPress;
  }

  return GestureEvent::kNone;
}

}  // namespace ui