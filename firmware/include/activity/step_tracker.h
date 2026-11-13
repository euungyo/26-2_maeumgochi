#pragma once

#include <stdint.h>

#include "hal/motion_sensor.h"

namespace activity {

struct StepTrackerConfig {
  // 마지막 걸음 이후 이 시간 안에 다음 걸음이 없으면 걷기가 멈춘 것으로 봅니다.
  // 센서 내장 카운터가 걸음을 묶어서 갱신할 수 있어 여유를 둡니다. 실측 후 조정합니다.
  uint32_t walkingTimeoutMs = 5000;
};

// 센서 내장 걸음 수 카운터를 받아 기준 시점 이후의 걸음 수, 걷는 중 여부,
// 걷기 지속시간을 계산합니다. Arduino에 의존하지 않아 PC에서 시험할 수 있습니다.
// 루프마다 update()를 호출하고, 미션을 시작할 때 reset()으로 기준 시점을 다시 잡습니다.
class StepTracker {
 public:
  explicit StepTracker(StepTrackerConfig config = StepTrackerConfig());

  // 걸음 수와 걷기 시간을 0으로 되돌립니다. 센서 카운터 자체는 건드리지 않습니다.
  void reset();

  void update(const hal::MotionReading& reading, uint32_t nowMs);

  // reset() 이후 걸음 수
  uint32_t steps() const { return steps_; }
  // reset() 이후 걸음이 이어진 시간(초). 멈춰 있던 시간은 포함하지 않습니다.
  uint32_t activeSeconds() const { return activeMs_ / 1000; }
  // 최근 walkingTimeoutMs 안에 걸음이 있었는지
  bool isWalking() const { return walking_; }
  // 마지막 update()에서 센서 값이 유효했는지
  bool isSensorValid() const { return sensorValid_; }

 private:
  StepTrackerConfig config_;

  bool hasSensorCount_ = false;  // 센서 카운터 기준값을 잡았는지
  uint32_t lastSensorCount_ = 0;

  uint32_t steps_ = 0;
  uint32_t activeMs_ = 0;

  bool hasStepTime_ = false;  // reset() 이후 걸음이 한 번이라도 있었는지
  uint32_t lastStepMs_ = 0;

  bool walking_ = false;
  bool sensorValid_ = false;
};

}  // namespace activity
