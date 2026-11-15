#include <unity.h>

#include "ui/nav_input_mapper.h"

namespace {

using hal::ButtonId;
using ui::NavInput;

struct Result {
  int count = 0;
  NavInput last = NavInput::kLeftShort;
};

// 5ms 간격으로 durationMs 동안 지정한 버튼만 누른 상태를 입력합니다.
uint32_t hold(ui::NavInputMapper& mapper, int buttonIndex, uint32_t startMs, uint32_t durationMs,
              Result& result) {
  uint32_t t = startMs;
  for (uint32_t elapsed = 0; elapsed <= durationMs; elapsed += 5) {
    t = startMs + elapsed;
    bool pressed[hal::kButtonCount] = {false, false, false};
    if (buttonIndex >= 0) pressed[buttonIndex] = true;
    NavInput input;
    if (mapper.update(pressed, t, input)) {
      ++result.count;
      result.last = input;
    }
  }
  return t;
}

void test_left_short_press() {
  ui::NavInputMapper mapper;
  Result r;
  uint32_t t = hold(mapper, 0, 0, 200, r);
  hold(mapper, -1, t + 5, 100, r);
  TEST_ASSERT_EQUAL_INT(1, r.count);
  TEST_ASSERT_EQUAL(NavInput::kLeftShort, r.last);
}

void test_center_short_press() {
  ui::NavInputMapper mapper;
  Result r;
  uint32_t t = hold(mapper, 1, 0, 200, r);
  hold(mapper, -1, t + 5, 100, r);
  TEST_ASSERT_EQUAL_INT(1, r.count);
  TEST_ASSERT_EQUAL(NavInput::kCenterShort, r.last);
}

void test_right_short_press() {
  ui::NavInputMapper mapper;
  Result r;
  uint32_t t = hold(mapper, 2, 0, 200, r);
  hold(mapper, -1, t + 5, 100, r);
  TEST_ASSERT_EQUAL_INT(1, r.count);
  TEST_ASSERT_EQUAL(NavInput::kRightShort, r.last);
}

void test_center_long_press_goes_back_once() {
  ui::NavInputMapper mapper;
  Result r;
  uint32_t t = hold(mapper, 1, 0, 1200, r);
  hold(mapper, -1, t + 5, 200, r);  // 떼도 짧게 누름이 추가로 나오면 안 됩니다.
  TEST_ASSERT_EQUAL_INT(1, r.count);
  TEST_ASSERT_EQUAL(NavInput::kCenterLong, r.last);
}

void test_side_long_press_is_ignored() {
  ui::NavInputMapper mapper;
  Result r;
  uint32_t t = hold(mapper, 0, 0, 1200, r);
  hold(mapper, -1, t + 5, 200, r);
  TEST_ASSERT_EQUAL_INT(0, r.count);
}

}  // namespace

extern "C" void setUp(void) {}
extern "C" void tearDown(void) {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_left_short_press);
  RUN_TEST(test_center_short_press);
  RUN_TEST(test_right_short_press);
  RUN_TEST(test_center_long_press_goes_back_once);
  RUN_TEST(test_side_long_press_is_ignored);
  return UNITY_END();
}