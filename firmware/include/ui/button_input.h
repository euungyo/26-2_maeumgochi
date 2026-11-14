#pragma once

#include <stdint.h>

#include "hal/buttons.h"
#include "ui/button_gesture.h"
#include "ui/screen_navigator.h"

namespace ui {

// 버튼 3개의 눌림 상태를 화면 이동 입력(NavInput)으로 바꿉니다.
// 왼쪽/오른쪽은 짧게 누름만, 가운데는 짧게/길게 누름을 사용합니다.
class ButtonInput {
 public:
  // 이번 호출에서 발생한 입력을 out에 채우고 개수를 반환합니다.
  // out은 hal::kButtonCount개 이상 크기여야 합니다.
  uint8_t poll(const hal::Buttons& buttons, uint32_t nowMs, NavInput* out);

 private:
  ButtonGesture gestures_[hal::kButtonCount];
};

}  // namespace ui