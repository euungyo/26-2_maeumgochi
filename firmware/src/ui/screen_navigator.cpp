#include "ui/screen_navigator.h"

namespace ui {

namespace {

// 메뉴 항목에 대응하는 화면입니다.
constexpr Screen kLifeMenuItems[] = {Screen::kMissionList, Screen::kCheckIn, Screen::kRecord};
constexpr Screen kCharacterMenuItems[] = {Screen::kStatus, Screen::kShop, Screen::kInventory};

}  // namespace

ScreenNavigator::ScreenNavigator()
    : current_(Screen::kCheckIn), menuIndex_(0), missionIndex_(0), missionReturn_(Screen::kMain) {}

uint8_t ScreenNavigator::menuItemCount(Screen screen) {
  switch (screen) {
    case Screen::kLifeMenu:
      return sizeof(kLifeMenuItems) / sizeof(kLifeMenuItems[0]);
    case Screen::kCharacterMenu:
      return sizeof(kCharacterMenuItems) / sizeof(kCharacterMenuItems[0]);
    case Screen::kMissionList:
      return kMissionSlotCount;
    default:
      return 0;
  }
}

Screen ScreenNavigator::parent() const {
  switch (current_) {
    case Screen::kLifeMenu:
    case Screen::kCharacterMenu:
      return Screen::kMain;
    case Screen::kMission:
      return missionReturn_;
    case Screen::kMissionList:
    case Screen::kRecord:
      return Screen::kLifeMenu;
    case Screen::kStatus:
    case Screen::kShop:
    case Screen::kInventory:
      return Screen::kCharacterMenu;
    default:
      return current_;  // 체크인과 메인에는 부모가 없습니다.
  }
}

void ScreenNavigator::goTo(Screen screen) {
  current_ = screen;
  menuIndex_ = 0;
}

void ScreenNavigator::setMissionIndex(uint8_t index) {
  missionIndex_ = index < kMissionSlotCount ? index : 0;
}

void ScreenNavigator::completeCheckIn() {
  if (current_ == Screen::kCheckIn) {
    goTo(Screen::kMain);
  }
}

bool ScreenNavigator::handle(NavInput input) {
  const Screen before = current_;

  // 체크인 화면의 입력은 CheckInFlow가 처리하므로 여기서는 이동하지 않습니다.
  if (current_ == Screen::kCheckIn) {
    return false;
  }

  if (input == NavInput::kCenterLong) {
    const bool backToList = current_ == Screen::kMission && parent() == Screen::kMissionList;
    goTo(parent());
    if (backToList) {
      menuIndex_ = missionIndex_;  // 목록에서 보던 줄을 그대로 강조합니다.
    }
    return current_ != before;
  }

  if (current_ == Screen::kMain) {
    if (input == NavInput::kLeftShort) goTo(Screen::kLifeMenu);
    if (input == NavInput::kRightShort) goTo(Screen::kCharacterMenu);
    if (input == NavInput::kCenterShort) {
      missionReturn_ = Screen::kMain;
      goTo(Screen::kMission);
    }
    return current_ != before;
  }

  if (current_ == Screen::kMission) {
    // 왼쪽/오른쪽은 이전/다음 미션입니다. 가운데(시작·완료)는 AppController가 처리합니다.
    if (input == NavInput::kLeftShort) {
      missionIndex_ = (missionIndex_ + kMissionSlotCount - 1) % kMissionSlotCount;
    } else if (input == NavInput::kRightShort) {
      missionIndex_ = (missionIndex_ + 1) % kMissionSlotCount;
    }
    return false;
  }

  const uint8_t count = menuItemCount(current_);
  if (count == 0) {
    return false;  // 상점, 기록 등 일반 화면은 자체 로직이 입력을 처리합니다.
  }

  if (input == NavInput::kLeftShort) {
    menuIndex_ = (menuIndex_ + count - 1) % count;
  } else if (input == NavInput::kRightShort) {
    menuIndex_ = (menuIndex_ + 1) % count;
  } else if (input == NavInput::kCenterShort) {
    if (current_ == Screen::kMissionList) {
      missionIndex_ = menuIndex_;
      missionReturn_ = Screen::kMissionList;
      goTo(Screen::kMission);
    } else {
      goTo(current_ == Screen::kLifeMenu ? kLifeMenuItems[menuIndex_]
                                         : kCharacterMenuItems[menuIndex_]);
    }
  }
  return current_ != before;
}

}  // namespace ui
