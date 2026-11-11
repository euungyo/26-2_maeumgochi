#include "ui/screen_navigator.h"

namespace ui {

namespace {

// 메뉴 항목에 대응하는 화면입니다.
constexpr Screen kLifeMenuItems[] = {Screen::kMissionList, Screen::kCheckIn,
                                     Screen::kRecord};
constexpr Screen kCharacterMenuItems[] = {Screen::kStatus, Screen::kShop, Screen::kInventory};

Screen parentOf(Screen screen) {
  switch (screen) {
    case Screen::kLifeMenu:
    case Screen::kCharacterMenu:
    case Screen::kMission:
      return Screen::kMain;
    case Screen::kMissionList:
    case Screen::kRecord:
      return Screen::kLifeMenu;
    case Screen::kStatus:
    case Screen::kShop:
    case Screen::kInventory:
      return Screen::kCharacterMenu;
    default:
      return screen;  // 체크인과 메인에는 부모가 없습니다.
  }
}

}  // namespace

ScreenNavigator::ScreenNavigator() : current_(Screen::kCheckIn), menuIndex_(0) {}

uint8_t ScreenNavigator::menuItemCount(Screen screen) {
  switch (screen) {
    case Screen::kLifeMenu:
      return sizeof(kLifeMenuItems) / sizeof(kLifeMenuItems[0]);
    case Screen::kCharacterMenu:
      return sizeof(kCharacterMenuItems) / sizeof(kCharacterMenuItems[0]);
    default:
      return 0;
  }
}

void ScreenNavigator::goTo(Screen screen) {
  current_ = screen;
  menuIndex_ = 0;
}

void ScreenNavigator::completeCheckIn() {
  if (current_ == Screen::kCheckIn) {
    goTo(Screen::kMain);
  }
}

bool ScreenNavigator::handle(NavInput input) {
  const Screen before = current_;

  // 체크인 화면의 입력은 체크인 화면 자체가 처리하므로 여기서는 이동하지 않습니다.
  if (current_ == Screen::kCheckIn) {
    return false;
  }

  if (input == NavInput::kCenterLong) {
    goTo(parentOf(current_));
    return current_ != before;
  }

  if (current_ == Screen::kMain) {
    if (input == NavInput::kLeftShort) goTo(Screen::kLifeMenu);
    if (input == NavInput::kCenterShort) goTo(Screen::kMission);
    if (input == NavInput::kRightShort) goTo(Screen::kCharacterMenu);
    return current_ != before;
  }

  const uint8_t count = menuItemCount(current_);
  if (count == 0) {
    return false;  // 미션, 상점 등 일반 화면은 자체 로직이 입력을 처리합니다.
  }

  if (input == NavInput::kLeftShort) {
    menuIndex_ = (menuIndex_ + count - 1) % count;
  } else if (input == NavInput::kRightShort) {
    menuIndex_ = (menuIndex_ + 1) % count;
  } else if (input == NavInput::kCenterShort) {
    const Screen next =
        (current_ == Screen::kLifeMenu) ? kLifeMenuItems[menuIndex_] : kCharacterMenuItems[menuIndex_];
    goTo(next);
  }
  return current_ != before;
}

}  // namespace ui
