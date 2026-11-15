#pragma once

#include <stdint.h>

#include "ui/button_gesture.h"

namespace ui {

enum class Screen : uint8_t {
  kCheckIn = 0,
  kMain,
  kLifeMenu,
  kCharacterMenu,
  kMission,
  kMissionList,
  kRecord,
  kStatus,
  kShop,
  kInventory,
};

enum class NavInput : uint8_t {
  kLeftShort = 0,
  kCenterShort,
  kRightShort,
  kCenterLong,
};

// 화면 이동 규칙만 담당합니다.
class ScreenNavigator {
 public:
  ScreenNavigator();

  Screen current() const { return current_; }
  // 현재 화면이 메뉴일 때 강조된 항목 번호입니다.
  uint8_t menuIndex() const { return menuIndex_; }

  // 메인 화면으로 이동합니다.
  void completeCheckIn();
  // 입력을 처리하고 화면이 바뀌었으면 true를 반환합니다.
  bool handle(NavInput input);

  static uint8_t menuItemCount(Screen screen);

 private:
  void goTo(Screen screen);

  Screen current_;
  uint8_t menuIndex_;
};

}  // namespace ui
