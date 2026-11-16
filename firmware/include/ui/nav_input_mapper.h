#pragma once

#include <stdint.h>

#include "hal/buttons.h"
#include "ui/button_gesture.h"
#include "ui/screen_navigator.h"

namespace ui {

// 버튼 3개의 눌림 상태를 ScreenNavigator 입력으로 바꿉니다.
// 왼쪽/오른쪽은 짧게 누름만, 가운데는 짧게(선택)와 길게(뒤로)를 사용합니다.
class NavInputMapper {
 public:
  // 루프마다 호출합니다. 이번 호출에서 입력이 발생하면 true를 반환하고 out에 담습니다.
  bool update(const bool pressed[hal::kButtonCount], uint32_t nowMs, NavInput& out);

 private:
  ButtonGesture gestures_[hal::kButtonCount];
};

}  // namespace ui