#include <unity.h>

#include <math.h>

#include "activity/ltr390_math.h"
#include "activity/sunlight_tracker.h"

namespace {

hal::LightReading reading(float lux, float uvi = 0.0f, bool valid = true) {
  return {lux, uvi, valid};
}

// 100ms 간격으로 durationMs 동안 같은 값을 넣고 마지막 시각을 반환합니다.
uint32_t feed(activity::SunlightTracker& tracker, const hal::LightReading& r, uint32_t startMs,
              uint32_t durationMs) {
  uint32_t t = startMs;
  for (uint32_t elapsed = 0; elapsed <= durationMs; elapsed += 100) {
    t = startMs + elapsed;
    tracker.update(r, t);
  }
  return t;
}

void test_starts_at_zero() {
  activity::SunlightTracker tracker;
  TEST_ASSERT_EQUAL_UINT32(0, tracker.seconds());
  TEST_ASSERT_FALSE(tracker.isSunny());
}

void test_bright_light_accumulates_time() {
  activity::SunlightTracker tracker;
  feed(tracker, reading(20000.0f), 0, 10000);
  TEST_ASSERT_EQUAL_UINT32(10, tracker.seconds());
  TEST_ASSERT_TRUE(tracker.isSunny());
}

void test_indoor_light_counts_nothing() {
  activity::SunlightTracker tracker;
  feed(tracker, reading(300.0f, 0.0f), 0, 60000);
  TEST_ASSERT_EQUAL_UINT32(0, tracker.seconds());
  TEST_ASSERT_FALSE(tracker.isSunny());
}

void test_time_stops_when_light_drops() {
  activity::SunlightTracker tracker;
  uint32_t t = feed(tracker, reading(20000.0f), 0, 4900);
  feed(tracker, reading(100.0f), t + 100, 5000);
  TEST_ASSERT_EQUAL_UINT32(5, tracker.seconds());
}

void test_uv_index_alone_can_qualify() {
  activity::SunlightTracker tracker;
  feed(tracker, reading(5000.0f, 4.0f), 0, 3000);
  TEST_ASSERT_EQUAL_UINT32(3, tracker.seconds());

  activity::SunlightConfig luxOnly;
  luxOnly.uvIndexThreshold = 0.0f;
  activity::SunlightTracker tracker2(luxOnly);
  feed(tracker2, reading(5000.0f, 4.0f), 0, 3000);
  TEST_ASSERT_EQUAL_UINT32(0, tracker2.seconds());
}

void test_invalid_sensor_counts_nothing() {
  activity::SunlightTracker tracker;
  feed(tracker, reading(50000.0f, 9.0f, false), 0, 5000);
  TEST_ASSERT_EQUAL_UINT32(0, tracker.seconds());
  TEST_ASSERT_FALSE(tracker.isSensorValid());
}

void test_long_gap_between_updates_is_not_counted() {
  activity::SunlightTracker tracker;
  tracker.update(reading(20000.0f), 0);
  tracker.update(reading(20000.0f), 60000);  // 루프가 1분 멈췄던 경우
  TEST_ASSERT_EQUAL_UINT32(0, tracker.seconds());
}

void test_reset_and_restore() {
  activity::SunlightTracker tracker;
  feed(tracker, reading(20000.0f), 0, 5000);
  tracker.reset();
  TEST_ASSERT_EQUAL_UINT32(0, tracker.seconds());

  tracker.restore(120);
  TEST_ASSERT_EQUAL_UINT32(120, tracker.seconds());
  feed(tracker, reading(20000.0f), 1000000, 2000);
  TEST_ASSERT_EQUAL_UINT32(122, tracker.seconds());
}

void test_millis_wraparound() {
  activity::SunlightTracker tracker;
  feed(tracker, reading(20000.0f), 0xFFFFFF00u, 1000);
  TEST_ASSERT_EQUAL_UINT32(1, tracker.seconds());
}

void test_ltr390_conversion_matches_datasheet_formula() {
  // gain 3, 16비트(25ms): lux = 0.6 * raw / (3 * 0.25) = 0.8 * raw
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 800.0f, activity::ltr390Lux(1000));
  // UVI = raw / (2300 * 3/18 * 25/400) = raw / 23.958
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, activity::ltr390Uvi(24));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, activity::ltr390Lux(0));
}

}  // namespace

extern "C" void setUp(void) {}
extern "C" void tearDown(void) {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_starts_at_zero);
  RUN_TEST(test_bright_light_accumulates_time);
  RUN_TEST(test_indoor_light_counts_nothing);
  RUN_TEST(test_time_stops_when_light_drops);
  RUN_TEST(test_uv_index_alone_can_qualify);
  RUN_TEST(test_invalid_sensor_counts_nothing);
  RUN_TEST(test_long_gap_between_updates_is_not_counted);
  RUN_TEST(test_reset_and_restore);
  RUN_TEST(test_millis_wraparound);
  RUN_TEST(test_ltr390_conversion_matches_datasheet_formula);
  return UNITY_END();
}
