#pragma once

#include <stdint.h>

namespace hal {

struct MotionReading {
  float accelX;        // 가속도, g
  float accelY;
  float accelZ;
  float gyroX;         // 각속도, deg/s
  float gyroY;
  float gyroZ;
  uint32_t stepCount;  // 센서 내장 걸음 수 카운터. 센서가 재시작하면 0부터 다시 셉니다.
  bool valid;          // 센서가 없거나 읽기 실패하면 false
};

class MotionSensor {
 public:
  virtual ~MotionSensor() = default;

  // 센서를 찾지 못하면 false를 반환합니다. 이 경우에도 앱은 계속 동작해야 합니다.
  virtual bool begin() = 0;
  // 새 값을 읽습니다. 호출 간격은 호출자가 정합니다.
  virtual void update() = 0;
  virtual bool isValid() const = 0;
  virtual MotionReading latest() const = 0;
};

}  // namespace hal
