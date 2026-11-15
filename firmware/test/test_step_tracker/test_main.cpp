#include <unity.h>

#include <stdint.h>

#include "activity/step_tracker.h"
#include "hal/motion_sensor.h"

namespace {

// 걸음 수를 원하는 대로 바꿀 수 있는 PC용 가짜 센서입니다.
class FakeMotionSensor : public hal::MotionSensor {
 public:
  bool begin() override { return connected; }
  void update() override {}
  bool isValid() const override { return connected; }
  hal::MotionReading latest() const override {
    return {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, stepCount, connected};
  }

  uint32_t stepCount = 0;
  bool connected = true;
};

void feed(activity::StepTracker& tracker, FakeMotionSensor& sensor, uint32_t nowMs) {
  sensor.update();
  tracker.update(sensor.latest(), nowMs);
}

void test_starts_at_zero() {
  activity::StepTracker tracker;

  TEST_ASSERT_EQUAL_UINT32(0, tracker.steps());
  TEST_ASSERT_EQUAL_UINT32(0, tracker.activeSeconds());
  TEST_ASSERT_FALSE(tracker.isWalking());
  TEST_ASSERT_FALSE(tracker.isSensorValid());
}

void test_steps_before_tracking_are_not_counted() {
  activity::StepTracker tracker;
  FakeMotionSensor sensor;
  sensor.stepCount = 500;  // 추적을 시작하기 전에 센서가 이미 센 걸음

  feed(tracker, sensor, 0);

  TEST_ASSERT_EQUAL_UINT32(0, tracker.steps());
  TEST_ASSERT_FALSE(tracker.isWalking());
  TEST_ASSERT_TRUE(tracker.isSensorValid());
}

void test_counts_new_steps() {
  activity::StepTracker tracker;
  FakeMotionSensor sensor;
  sensor.stepCount = 500;
  feed(tracker, sensor, 0);

  sensor.stepCount = 502;
  feed(tracker, sensor, 1000);
  sensor.stepCount = 510;
  feed(tracker, sensor, 2000);

  TEST_ASSERT_EQUAL_UINT32(10, tracker.steps());
  TEST_ASSERT_TRUE(tracker.isWalking());
}

void test_active_time_accumulates_while_steps_continue() {
  activity::StepTracker tracker;
  FakeMotionSensor sensor;
  feed(tracker, sensor, 0);

  // 1초마다 2걸음씩 60초 동안 걷기
  for (uint32_t second = 1; second <= 60; ++second) {
    sensor.stepCount += 2;
    feed(tracker, sensor, second * 1000);
  }

  TEST_ASSERT_EQUAL_UINT32(120, tracker.steps());
  // 첫 걸음은 구간의 시작점이므로 59초가 쌓입니다.
  TEST_ASSERT_EQUAL_UINT32(59, tracker.activeSeconds());
}

void test_walking_stops_after_timeout() {
  activity::StepTrackerConfig config;
  config.walkingTimeoutMs = 5000;
  activity::StepTracker tracker(config);
  FakeMotionSensor sensor;
  feed(tracker, sensor, 0);
  sensor.stepCount = 2;
  feed(tracker, sensor, 1000);

  feed(tracker, sensor, 6000);  // 마지막 걸음에서 정확히 5초
  TEST_ASSERT_TRUE(tracker.isWalking());

  feed(tracker, sensor, 6001);
  TEST_ASSERT_FALSE(tracker.isWalking());
}

void test_pause_is_not_counted_as_active_time() {
  activity::StepTracker tracker;
  FakeMotionSensor sensor;
  feed(tracker, sensor, 0);

  // 10초 걷기 (0초에 시작해 10초까지)
  for (uint32_t second = 0; second <= 10; ++second) {
    sensor.stepCount += 2;
    feed(tracker, sensor, second * 1000);
  }
  TEST_ASSERT_EQUAL_UINT32(10, tracker.activeSeconds());

  // 5분 멈춤
  feed(tracker, sensor, 310000);
  TEST_ASSERT_FALSE(tracker.isWalking());

  // 다시 10초 걷기
  for (uint32_t second = 0; second <= 10; ++second) {
    sensor.stepCount += 2;
    feed(tracker, sensor, 400000 + second * 1000);
  }

  TEST_ASSERT_EQUAL_UINT32(44, tracker.steps());
  TEST_ASSERT_EQUAL_UINT32(20, tracker.activeSeconds());
}

void test_standing_still_counts_nothing() {
  activity::StepTracker tracker;
  FakeMotionSensor sensor;
  sensor.stepCount = 1234;

  // 정지 상태 5분: 100ms마다 읽어도 카운터가 그대로면 아무것도 늘지 않습니다.
  for (uint32_t ms = 0; ms <= 300000; ms += 100) {
    feed(tracker, sensor, ms);
  }

  TEST_ASSERT_EQUAL_UINT32(0, tracker.steps());
  TEST_ASSERT_EQUAL_UINT32(0, tracker.activeSeconds());
  TEST_ASSERT_FALSE(tracker.isWalking());
}

void test_reset_starts_a_new_baseline() {
  activity::StepTracker tracker;
  FakeMotionSensor sensor;
  feed(tracker, sensor, 0);
  sensor.stepCount = 300;
  feed(tracker, sensor, 1000);
  sensor.stepCount = 302;
  feed(tracker, sensor, 2000);

  tracker.reset();  // 미션 시작

  TEST_ASSERT_EQUAL_UINT32(0, tracker.steps());
  TEST_ASSERT_EQUAL_UINT32(0, tracker.activeSeconds());
  TEST_ASSERT_FALSE(tracker.isWalking());

  // reset() 이전 걸음이 다시 더해지지 않습니다.
  feed(tracker, sensor, 3000);
  TEST_ASSERT_EQUAL_UINT32(0, tracker.steps());

  sensor.stepCount = 350;
  feed(tracker, sensor, 4000);
  TEST_ASSERT_EQUAL_UINT32(48, tracker.steps());
}

void test_invalid_reading_is_ignored() {
  activity::StepTracker tracker;
  FakeMotionSensor sensor;
  feed(tracker, sensor, 0);
  sensor.stepCount = 10;
  feed(tracker, sensor, 1000);

  // 센서가 빠진 동안 엉뚱한 값이 와도 걸음 수는 그대로입니다.
  sensor.connected = false;
  sensor.stepCount = 99999;
  feed(tracker, sensor, 2000);

  TEST_ASSERT_FALSE(tracker.isSensorValid());
  TEST_ASSERT_EQUAL_UINT32(10, tracker.steps());
}

void test_sensor_restart_keeps_counted_steps() {
  activity::StepTracker tracker;
  FakeMotionSensor sensor;
  sensor.stepCount = 1000;
  feed(tracker, sensor, 0);
  sensor.stepCount = 1100;
  feed(tracker, sensor, 1000);
  TEST_ASSERT_EQUAL_UINT32(100, tracker.steps());

  // 센서가 재연결되어 카운터가 0부터 다시 시작
  sensor.connected = false;
  feed(tracker, sensor, 2000);
  sensor.connected = true;
  sensor.stepCount = 0;
  feed(tracker, sensor, 3000);
  TEST_ASSERT_EQUAL_UINT32(100, tracker.steps());

  sensor.stepCount = 20;
  feed(tracker, sensor, 4000);
  TEST_ASSERT_EQUAL_UINT32(120, tracker.steps());
}

void test_sensor_missing_from_start_does_not_break() {
  activity::StepTracker tracker;
  FakeMotionSensor sensor;
  sensor.connected = false;

  TEST_ASSERT_FALSE(sensor.begin());
  for (uint32_t ms = 0; ms < 10000; ms += 1000) {
    feed(tracker, sensor, ms);
  }

  TEST_ASSERT_FALSE(tracker.isSensorValid());
  TEST_ASSERT_EQUAL_UINT32(0, tracker.steps());
  TEST_ASSERT_FALSE(tracker.isWalking());
}

void test_millis_wraparound_is_handled() {
  activity::StepTracker tracker;
  FakeMotionSensor sensor;
  const uint32_t nearWrap = 0xFFFFFFFFu - 1500;  // millis()가 넘치기 1.5초 전
  feed(tracker, sensor, nearWrap);

  sensor.stepCount = 2;
  feed(tracker, sensor, nearWrap + 500);
  sensor.stepCount = 4;
  feed(tracker, sensor, nearWrap + 2500);  // 넘친 뒤 (작은 값)

  TEST_ASSERT_TRUE(tracker.isWalking());
  TEST_ASSERT_EQUAL_UINT32(4, tracker.steps());
  TEST_ASSERT_EQUAL_UINT32(2, tracker.activeSeconds());
}

}  // namespace

extern "C" void setUp(void) {}
extern "C" void tearDown(void) {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_starts_at_zero);
  RUN_TEST(test_steps_before_tracking_are_not_counted);
  RUN_TEST(test_counts_new_steps);
  RUN_TEST(test_active_time_accumulates_while_steps_continue);
  RUN_TEST(test_walking_stops_after_timeout);
  RUN_TEST(test_pause_is_not_counted_as_active_time);
  RUN_TEST(test_standing_still_counts_nothing);
  RUN_TEST(test_reset_starts_a_new_baseline);
  RUN_TEST(test_invalid_reading_is_ignored);
  RUN_TEST(test_sensor_restart_keeps_counted_steps);
  RUN_TEST(test_sensor_missing_from_start_does_not_break);
  RUN_TEST(test_millis_wraparound_is_handled);
  return UNITY_END();
}
