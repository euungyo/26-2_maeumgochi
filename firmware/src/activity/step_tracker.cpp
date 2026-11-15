#include "activity/step_tracker.h"

namespace activity {

StepTracker::StepTracker(StepTrackerConfig config) : config_(config) {}

void StepTracker::reset() {
  steps_ = 0;
  activeMs_ = 0;
  hasStepTime_ = false;
  walking_ = false;
  // lastSensorCount_는 유지합니다. reset() 이전에 센 걸음이 다음 update()에서 다시 더해지면 안 됩니다.
}

void StepTracker::update(const hal::MotionReading& reading, uint32_t nowMs) {
  sensorValid_ = reading.valid;

  if (reading.valid) {
    if (!hasSensorCount_) {
      // 첫 유효 값은 기준으로만 삼습니다. 추적을 시작하기 전에 센 걸음은 포함하지 않습니다.
      hasSensorCount_ = true;
      lastSensorCount_ = reading.stepCount;
    } else if (reading.stepCount < lastSensorCount_) {
      // 센서가 재시작해 카운터가 0부터 다시 시작한 경우입니다.
      // 끊긴 동안의 걸음은 알 수 없으므로 지금까지 센 값은 유지하고 기준만 다시 잡습니다.
      lastSensorCount_ = reading.stepCount;
    } else if (reading.stepCount > lastSensorCount_) {
      steps_ += reading.stepCount - lastSensorCount_;
      lastSensorCount_ = reading.stepCount;

      // 직전 걸음에서 이어진 경우에만 그 사이 시간을 걷기 시간으로 더합니다.
      // uint32_t 뺄셈이라 millis()가 한 바퀴 돌아도 간격이 올바르게 계산됩니다.
      if (hasStepTime_ && nowMs - lastStepMs_ <= config_.walkingTimeoutMs) {
        activeMs_ += nowMs - lastStepMs_;
      }
      hasStepTime_ = true;
      lastStepMs_ = nowMs;
    }
  }

  // 센서가 잠깐 끊겨도 마지막 걸음 시각 기준으로 판단합니다.
  walking_ = hasStepTime_ && nowMs - lastStepMs_ <= config_.walkingTimeoutMs;
  if (!walking_) {
    // 걷기가 끊기면 다음 걸음은 새 구간의 시작으로 봅니다.
    // 오래 멈춘 뒤 millis()가 한 바퀴 돌아 간격이 짧게 계산되는 경우도 막습니다.
    hasStepTime_ = false;
  }
}

}  // namespace activity
