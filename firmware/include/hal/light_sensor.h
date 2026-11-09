#pragma once

namespace hal {

struct LightReading {
  float lux;       // 주변 밝기
  float uvIndex;   // 자외선 지수
  bool valid;      // 센서가 없거나 읽기 실패하면 false
};

class LightSensor {
 public:
  virtual ~LightSensor() = default;

  // 센서를 찾지 못하면 false를 반환합니다. 이 경우에도 앱은 계속 동작해야 합니다.
  virtual bool begin() = 0;
  // 새 값을 측정합니다. 측정에 시간이 걸리므로 호출 간격은 호출자가 정합니다.
  virtual void update() = 0;
  virtual bool isValid() const = 0;
  virtual LightReading latest() const = 0;
};

}  // namespace hal