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

// 하루에 배정되는 미션 수입니다. (오전, 오후, 특수)
constexpr uint8_t kMissionSlotCount = 3;

// 화면 이동 규칙만 담당합니다.
class ScreenNavigator {
 public:
  ScreenNavigator();

  Screen current() const { return current_; }
  // 현재 화면이 메뉴(또는 미션 목록)일 때 강조된 항목 번호입니다.
  uint8_t menuIndex() const { return menuIndex_; }
  // 미션 상세 화면이 보여 줄 미션 번호입니다.
  uint8_t missionIndex() const { return missionIndex_; }
  // 메인에서 가운데 버튼을 누르기 전에 "현재 미션"을 지정할 때 씁니다.
  void setMissionIndex(uint8_t index);

  // 메인 화면으로 이동합니다.
  void completeCheckIn();
  // 입력을 처리하고 화면이 바뀌었으면 true를 반환합니다.
  bool handle(NavInput input);

  static uint8_t menuItemCount(Screen screen);

 private:
  void goTo(Screen screen);
  Screen parent() const;

  Screen current_;
  uint8_t menuIndex_;
  uint8_t missionIndex_;
  Screen missionReturn_;  // 미션 상세에서 뒤로 가면 돌아갈 화면 (메인 또는 미션 목록)
};

}  // namespace ui
