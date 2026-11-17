#include <unity.h>

#include "core/mission_generator.h"
#include "ui/checkin_flow.h"

namespace {

using ui::CheckInFlow;
using ui::CheckInResult;
using ui::NavInput;

void test_starts_at_mood_step_with_neutral_value() {
  CheckInFlow flow;
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInFlow::Step::kMood), static_cast<int>(flow.step()));
  TEST_ASSERT_EQUAL_UINT8(3, flow.value());
}

void test_left_and_right_change_value() {
  CheckInFlow flow;
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInResult::kUpdated),
                        static_cast<int>(flow.handle(NavInput::kRightShort)));
  TEST_ASSERT_EQUAL_UINT8(4, flow.value());
  flow.handle(NavInput::kLeftShort);
  flow.handle(NavInput::kLeftShort);
  TEST_ASSERT_EQUAL_UINT8(2, flow.value());
}

void test_value_is_clamped_to_one_through_five() {
  CheckInFlow flow;
  for (int i = 0; i < 10; ++i) flow.handle(NavInput::kRightShort);
  TEST_ASSERT_EQUAL_UINT8(5, flow.value());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInResult::kNone),
                        static_cast<int>(flow.handle(NavInput::kRightShort)));
  for (int i = 0; i < 10; ++i) flow.handle(NavInput::kLeftShort);
  TEST_ASSERT_EQUAL_UINT8(1, flow.value());
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInResult::kNone),
                        static_cast<int>(flow.handle(NavInput::kLeftShort)));
}

void test_confirm_moves_to_energy_step_and_resets_value() {
  CheckInFlow flow;
  flow.handle(NavInput::kLeftShort);
  flow.handle(NavInput::kLeftShort);  // 기분 1
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInResult::kUpdated),
                        static_cast<int>(flow.handle(NavInput::kCenterShort)));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInFlow::Step::kEnergy), static_cast<int>(flow.step()));
  TEST_ASSERT_EQUAL_UINT8(3, flow.value());
}

void test_second_confirm_completes_with_both_values() {
  CheckInFlow flow;
  flow.handle(NavInput::kLeftShort);
  flow.handle(NavInput::kLeftShort);
  flow.handle(NavInput::kCenterShort);  // 기분 1
  flow.handle(NavInput::kRightShort);
  flow.handle(NavInput::kRightShort);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInResult::kCompleted),
                        static_cast<int>(flow.handle(NavInput::kCenterShort)));  // 에너지 5
  TEST_ASSERT_EQUAL_UINT8(1, flow.mood());
  TEST_ASSERT_EQUAL_UINT8(5, flow.energy());
  // 이 값이 미션 난이도 3(필수 테스트 mood=1, energy=5)으로 이어집니다.
  TEST_ASSERT_EQUAL_UINT8(3, *maeumgochi::calculateMissionDifficulty(flow.mood(), flow.energy()));
}

void test_long_press_on_energy_step_goes_back_to_mood() {
  CheckInFlow flow;
  flow.handle(NavInput::kRightShort);  // 기분 4
  flow.handle(NavInput::kCenterShort);
  flow.handle(NavInput::kLeftShort);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInResult::kUpdated),
                        static_cast<int>(flow.handle(NavInput::kCenterLong)));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInFlow::Step::kMood), static_cast<int>(flow.step()));
  TEST_ASSERT_EQUAL_UINT8(4, flow.value());
}

void test_long_press_on_first_step_is_ignored_unless_cancel_allowed() {
  CheckInFlow flow;
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInResult::kNone),
                        static_cast<int>(flow.handle(NavInput::kCenterLong)));

  flow.reset(true);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInResult::kCancelled),
                        static_cast<int>(flow.handle(NavInput::kCenterLong)));
}

void test_reset_starts_over() {
  CheckInFlow flow;
  flow.handle(NavInput::kRightShort);
  flow.handle(NavInput::kCenterShort);
  flow.reset(false);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(CheckInFlow::Step::kMood), static_cast<int>(flow.step()));
  TEST_ASSERT_EQUAL_UINT8(3, flow.value());
}

}  // namespace

extern "C" void setUp(void) {}
extern "C" void tearDown(void) {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_starts_at_mood_step_with_neutral_value);
  RUN_TEST(test_left_and_right_change_value);
  RUN_TEST(test_value_is_clamped_to_one_through_five);
  RUN_TEST(test_confirm_moves_to_energy_step_and_resets_value);
  RUN_TEST(test_second_confirm_completes_with_both_values);
  RUN_TEST(test_long_press_on_energy_step_goes_back_to_mood);
  RUN_TEST(test_long_press_on_first_step_is_ignored_unless_cancel_allowed);
  RUN_TEST(test_reset_starts_over);
  return UNITY_END();
}
