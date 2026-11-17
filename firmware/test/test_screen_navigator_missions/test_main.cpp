#include <unity.h>

#include "ui/screen_navigator.h"

namespace {

using ui::NavInput;
using ui::Screen;

ui::ScreenNavigator mainScreenNavigator() {
  ui::ScreenNavigator nav;
  nav.completeCheckIn();
  return nav;
}

// 메인 -> 생활 메뉴 -> 미션 목록
ui::ScreenNavigator missionListNavigator() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.handle(NavInput::kLeftShort);
  nav.handle(NavInput::kCenterShort);
  return nav;
}

void test_mission_list_has_one_row_per_slot() {
  TEST_ASSERT_EQUAL_UINT8(ui::kMissionSlotCount,
                          ui::ScreenNavigator::menuItemCount(Screen::kMissionList));
}

void test_mission_list_cursor_wraps() {
  ui::ScreenNavigator nav = missionListNavigator();
  nav.handle(NavInput::kLeftShort);
  TEST_ASSERT_EQUAL_UINT8(2, nav.menuIndex());
  nav.handle(NavInput::kRightShort);
  TEST_ASSERT_EQUAL_UINT8(0, nav.menuIndex());
}

void test_selecting_a_row_opens_that_mission() {
  ui::ScreenNavigator nav = missionListNavigator();
  nav.handle(NavInput::kRightShort);
  nav.handle(NavInput::kCenterShort);
  TEST_ASSERT_EQUAL(Screen::kMission, nav.current());
  TEST_ASSERT_EQUAL_UINT8(1, nav.missionIndex());
}

void test_back_from_mission_opened_from_list_returns_to_list_with_cursor() {
  ui::ScreenNavigator nav = missionListNavigator();
  nav.handle(NavInput::kRightShort);
  nav.handle(NavInput::kRightShort);
  nav.handle(NavInput::kCenterShort);  // 세 번째 미션
  nav.handle(NavInput::kCenterLong);
  TEST_ASSERT_EQUAL(Screen::kMissionList, nav.current());
  TEST_ASSERT_EQUAL_UINT8(2, nav.menuIndex());
  nav.handle(NavInput::kCenterLong);
  TEST_ASSERT_EQUAL(Screen::kLifeMenu, nav.current());
}

void test_back_from_mission_opened_from_main_returns_to_main() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.setMissionIndex(1);
  nav.handle(NavInput::kCenterShort);
  TEST_ASSERT_EQUAL(Screen::kMission, nav.current());
  TEST_ASSERT_EQUAL_UINT8(1, nav.missionIndex());
  nav.handle(NavInput::kCenterLong);
  TEST_ASSERT_EQUAL(Screen::kMain, nav.current());
}

void test_left_right_in_mission_detail_switch_mission() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.handle(NavInput::kCenterShort);
  TEST_ASSERT_EQUAL_UINT8(0, nav.missionIndex());
  nav.handle(NavInput::kLeftShort);
  TEST_ASSERT_EQUAL_UINT8(2, nav.missionIndex());
  nav.handle(NavInput::kRightShort);
  nav.handle(NavInput::kRightShort);
  TEST_ASSERT_EQUAL_UINT8(1, nav.missionIndex());
  TEST_ASSERT_EQUAL(Screen::kMission, nav.current());
}

void test_out_of_range_mission_index_is_ignored() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.setMissionIndex(9);
  TEST_ASSERT_EQUAL_UINT8(0, nav.missionIndex());
}

}  // namespace

extern "C" void setUp(void) {}
extern "C" void tearDown(void) {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_mission_list_has_one_row_per_slot);
  RUN_TEST(test_mission_list_cursor_wraps);
  RUN_TEST(test_selecting_a_row_opens_that_mission);
  RUN_TEST(test_back_from_mission_opened_from_list_returns_to_list_with_cursor);
  RUN_TEST(test_back_from_mission_opened_from_main_returns_to_main);
  RUN_TEST(test_left_right_in_mission_detail_switch_mission);
  RUN_TEST(test_out_of_range_mission_index_is_ignored);
  return UNITY_END();
}
