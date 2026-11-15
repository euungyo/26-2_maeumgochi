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

void test_boot_starts_at_checkin() {
  ui::ScreenNavigator nav;
  TEST_ASSERT_EQUAL(Screen::kCheckIn, nav.current());
}

void test_checkin_ignores_navigation_inputs() {
  ui::ScreenNavigator nav;
  TEST_ASSERT_FALSE(nav.handle(NavInput::kCenterLong));
  TEST_ASSERT_EQUAL(Screen::kCheckIn, nav.current());
}

void test_complete_checkin_goes_to_main() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  TEST_ASSERT_EQUAL(Screen::kMain, nav.current());
}

void test_main_buttons_open_screens() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.handle(NavInput::kLeftShort);
  TEST_ASSERT_EQUAL(Screen::kLifeMenu, nav.current());

  nav = mainScreenNavigator();
  nav.handle(NavInput::kCenterShort);
  TEST_ASSERT_EQUAL(Screen::kMission, nav.current());

  nav = mainScreenNavigator();
  nav.handle(NavInput::kRightShort);
  TEST_ASSERT_EQUAL(Screen::kCharacterMenu, nav.current());
}

void test_center_long_goes_back_to_main() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.handle(NavInput::kCenterShort);
  TEST_ASSERT_TRUE(nav.handle(NavInput::kCenterLong));
  TEST_ASSERT_EQUAL(Screen::kMain, nav.current());
}

void test_back_on_main_does_nothing() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  TEST_ASSERT_FALSE(nav.handle(NavInput::kCenterLong));
  TEST_ASSERT_EQUAL(Screen::kMain, nav.current());
}

void test_menu_selection_wraps_around() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.handle(NavInput::kRightShort);  // 캐릭터 메뉴, 항목 3개
  nav.handle(NavInput::kLeftShort);
  TEST_ASSERT_EQUAL_UINT8(2, nav.menuIndex());
  nav.handle(NavInput::kRightShort);
  TEST_ASSERT_EQUAL_UINT8(0, nav.menuIndex());
}

void test_select_menu_item_opens_shop_and_back_returns_to_menu() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.handle(NavInput::kRightShort);
  nav.handle(NavInput::kRightShort);  // 상점
  nav.handle(NavInput::kCenterShort);
  TEST_ASSERT_EQUAL(Screen::kShop, nav.current());

  nav.handle(NavInput::kCenterLong);
  TEST_ASSERT_EQUAL(Screen::kCharacterMenu, nav.current());
  nav.handle(NavInput::kCenterLong);
  TEST_ASSERT_EQUAL(Screen::kMain, nav.current());
}

void test_life_menu_opens_mission_list() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.handle(NavInput::kLeftShort);
  nav.handle(NavInput::kCenterShort);
  TEST_ASSERT_EQUAL(Screen::kMissionList, nav.current());
}

void test_life_menu_opens_record_screen() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.handle(NavInput::kLeftShort);
  nav.handle(NavInput::kRightShort);
  nav.handle(NavInput::kRightShort);
  nav.handle(NavInput::kCenterShort);
  TEST_ASSERT_EQUAL(Screen::kRecord, nav.current());

  nav.handle(NavInput::kCenterLong);
  TEST_ASSERT_EQUAL(Screen::kLifeMenu, nav.current());
}

void test_menu_index_resets_after_leaving() {
  ui::ScreenNavigator nav = mainScreenNavigator();
  nav.handle(NavInput::kRightShort);
  nav.handle(NavInput::kRightShort);
  nav.handle(NavInput::kCenterLong);
  nav.handle(NavInput::kRightShort);
  TEST_ASSERT_EQUAL_UINT8(0, nav.menuIndex());
}

}  // namespace

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_boot_starts_at_checkin);
  RUN_TEST(test_checkin_ignores_navigation_inputs);
  RUN_TEST(test_complete_checkin_goes_to_main);
  RUN_TEST(test_main_buttons_open_screens);
  RUN_TEST(test_center_long_goes_back_to_main);
  RUN_TEST(test_back_on_main_does_nothing);
  RUN_TEST(test_menu_selection_wraps_around);
  RUN_TEST(test_select_menu_item_opens_shop_and_back_returns_to_menu);
  RUN_TEST(test_life_menu_opens_mission_list);
  RUN_TEST(test_life_menu_opens_record_screen);
  RUN_TEST(test_menu_index_resets_after_leaving);
  return UNITY_END();
}
