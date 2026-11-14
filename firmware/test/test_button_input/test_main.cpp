#include <unity.h>

#include "ui/button_input.h"

namespace {

class FakeButtons : public hal::Buttons {
 public:
  bool begin() override { return true; }
  void update() override {}
  bool isPressed(hal::ButtonId id) const override { return pressed[static_cast<uint8_t>(id)]; }
  bool pressed[hal::kButtonCount] = {};
};

struct Collected {
  int count = 0;
  ui::NavInput last = ui::NavInput::kLeftShort;
};

// 5ms 간격으로 durationMs 동안 입력하고 마지막 시각을 반환합니다.
uint32_t run(ui::ButtonInput& input, FakeButtons& fake, uint32_t startMs, uint32_t durationMs,
             Collected& result) {
  uint32_t t = startMs;
  for (uint32_t elapsed = 0; elapsed <= durationMs; elapsed += 5) {
    t = startMs + elapsed;
    ui::NavInput events[hal::kButtonCount];
    const uint8_t n = input.poll(fake, t, events);
    for (uint8_t i = 0; i < n; ++i) {
      ++result.count;
      result.last = events[i];
    }
  }
  return t;
}

void test_left_short_press() {
  ui::ButtonInput input;
  FakeButtons fake;
  Collected c;
  fake.pressed[0] = true;
  uint32_t t = run(input, fake, 0, 200, c);
  fake.pressed[0] = false;
  run(input, fake, t + 5, 100, c);
  TEST_ASSERT_EQUAL_INT(1, c.count);
  TEST_ASSERT_EQUAL(ui::NavInput::kLeftShort, c.last);
}

void test_right_short_press() {
  ui::ButtonInput input;
  FakeButtons fake;
  Collected c;
  fake.pressed[2] = true;
  uint32_t t = run(input, fake, 0, 200, c);
  fake.pressed[2] = false;
  run(input, fake, t + 5, 100, c);
  TEST_ASSERT_EQUAL_INT(1, c.count);
  TEST_ASSERT_EQUAL(ui::NavInput::kRightShort, c.last);
}

void test_center_short_press() {
  ui::ButtonInput input;
  FakeButtons fake;
  Collected c;
  fake.pressed[1] = true;
  uint32_t t = run(input, fake, 0, 200, c);
  fake.pressed[1] = false;
  run(input, fake, t + 5, 100, c);
  TEST_ASSERT_EQUAL_INT(1, c.count);
  TEST_ASSERT_EQUAL(ui::NavInput::kCenterShort, c.last);
}

void test_center_long_press_fires_once_and_no_short_on_release() {
  ui::ButtonInput input;
  FakeButtons fake;
  Collected c;
  fake.pressed[1] = true;
  uint32_t t = run(input, fake, 0, 1200, c);
  TEST_ASSERT_EQUAL_INT(1, c.count);
  TEST_ASSERT_EQUAL(ui::NavInput::kCenterLong, c.last);
  fake.pressed[1] = false;
  run(input, fake, t + 5, 200, c);
  TEST_ASSERT_EQUAL_INT(1, c.count);
}

void test_side_buttons_ignore_long_press() {
  ui::ButtonInput input;
  FakeButtons fake;
  Collected c;
  fake.pressed[0] = true;
  uint32_t t = run(input, fake, 0, 1200, c);
  fake.pressed[0] = false;
  run(input, fake, t + 5, 200, c);
  TEST_ASSERT_EQUAL_INT(0, c.count);
}

}  // namespace

extern "C" void setUp(void) {}
extern "C" void tearDown(void) {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_left_short_press);
  RUN_TEST(test_right_short_press);
  RUN_TEST(test_center_short_press);
  RUN_TEST(test_center_long_press_fires_once_and_no_short_on_release);
  RUN_TEST(test_side_buttons_ignore_long_press);
  return UNITY_END();
}