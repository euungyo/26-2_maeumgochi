#include <unity.h>

#include "ui/button_gesture.h"

namespace {

struct Counts {
  int shortCount = 0;
  int longCount = 0;
};

// 5ms 간격으로 raw 상태를 durationMs 동안 입력하고 마지막 시각을 반환합니다.
uint32_t feed(ui::ButtonGesture& g, bool raw, uint32_t startMs, uint32_t durationMs,
              Counts& counts) {
  uint32_t t = startMs;
  for (uint32_t elapsed = 0; elapsed <= durationMs; elapsed += 5) {
    t = startMs + elapsed;
    const ui::GestureEvent ev = g.update(raw, t);
    if (ev == ui::GestureEvent::kShortPress) ++counts.shortCount;
    if (ev == ui::GestureEvent::kLongPress) ++counts.longCount;
  }
  return t;
}

void test_short_press_fires_on_release() {
  ui::ButtonGesture g;
  Counts c;
  uint32_t t = feed(g, true, 0, 200, c);
  TEST_ASSERT_EQUAL_INT(0, c.shortCount);
  feed(g, false, t + 5, 100, c);
  TEST_ASSERT_EQUAL_INT(1, c.shortCount);
  TEST_ASSERT_EQUAL_INT(0, c.longCount);
}

void test_bounce_shorter_than_debounce_is_ignored() {
  ui::ButtonGesture g;
  Counts c;
  uint32_t t = feed(g, true, 0, 10, c);
  feed(g, false, t + 5, 200, c);
  TEST_ASSERT_EQUAL_INT(0, c.shortCount);
  TEST_ASSERT_EQUAL_INT(0, c.longCount);
}

void test_long_press_not_fired_before_threshold() {
  ui::ButtonGesture g;
  Counts c;
  feed(g, true, 0, 800, c);
  TEST_ASSERT_EQUAL_INT(0, c.longCount);
}

void test_long_press_fires_once_while_held() {
  ui::ButtonGesture g;
  Counts c;
  feed(g, true, 0, 2000, c);
  TEST_ASSERT_EQUAL_INT(1, c.longCount);
}

void test_no_short_press_after_long_press_release() {
  ui::ButtonGesture g;
  Counts c;
  uint32_t t = feed(g, true, 0, 1200, c);
  feed(g, false, t + 5, 200, c);
  TEST_ASSERT_EQUAL_INT(1, c.longCount);
  TEST_ASSERT_EQUAL_INT(0, c.shortCount);
}

void test_two_presses_are_counted_separately() {
  ui::ButtonGesture g;
  Counts c;
  uint32_t t = feed(g, true, 0, 200, c);
  t = feed(g, false, t + 5, 200, c);
  t = feed(g, true, t + 5, 200, c);
  feed(g, false, t + 5, 200, c);
  TEST_ASSERT_EQUAL_INT(2, c.shortCount);
}

void test_millis_wraparound() {
  ui::ButtonGesture g;
  Counts c;
  uint32_t t = feed(g, true, 0xFFFFFF00u, 1200, c);
  TEST_ASSERT_EQUAL_INT(1, c.longCount);
  feed(g, false, t + 5, 200, c);
  TEST_ASSERT_EQUAL_INT(0, c.shortCount);
}

}  // namespace

extern "C" {
void setUp(void) {}
void tearDown(void) {}
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_short_press_fires_on_release);
  RUN_TEST(test_bounce_shorter_than_debounce_is_ignored);
  RUN_TEST(test_long_press_not_fired_before_threshold);
  RUN_TEST(test_long_press_fires_once_while_held);
  RUN_TEST(test_no_short_press_after_long_press_release);
  RUN_TEST(test_two_presses_are_counted_separately);
  RUN_TEST(test_millis_wraparound);
  return UNITY_END();
}