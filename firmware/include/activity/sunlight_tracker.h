#pragma once

#include <stdint.h>

#include "hal/light_sensor.h"

namespace activity {

struct SunlightConfig {
  // 햇빛으로 인정하는 기준입니다. 값은 임시이며 sunlight_check로 실측해 정합니다. (docs/api.md)
  float luxThreshold = 10000.0f;
  // UV 지수 기준입니다. 0 이하면 조도만 봅니다. 조도와 UV 중 하나라도 넘으면 햇빛입니다.
  float uvIndexThreshold = 3.0f;
  // update() 호출 간격이 이보다 길면 그 구간은 세지 않습니다. (루프가 멈췄던 시간을 햇빛으로 보지 않기 위함)
  uint32_t maxGapMs = 2000;
};

// 조도 센서 값을 받아 햇빛을 쬔 시간을 누적합니다. Arduino에 의존하지 않아 PC에서 시험할 수 있습니다.
// 루프마다 update()를 호출합니다. 하루가 바뀌면 reset()을 호출합니다.
class SunlightTracker {
 public:
  explicit SunlightTracker(SunlightConfig config = SunlightConfig());

  void reset();
  // 저장된 누적 시간을 되돌립니다. (재부팅 복원용)
  void restore(uint32_t seconds);

  void update(const hal::LightReading& reading, uint32_t nowMs);

  uint32_t seconds() const { return sunMs_ / 1000; }
  // 마지막 측정이 햇빛 기준을 넘었는지
  bool isSunny() const { return sunny_; }
  bool isSensorValid() const { return sensorValid_; }

 private:
  bool meetsThreshold(const hal::LightReading& reading) const;

  SunlightConfig config_;
  uint32_t sunMs_ = 0;
  bool hasLast_ = false;
  uint32_t lastMs_ = 0;
  bool sunny_ = false;
  bool sensorValid_ = false;
};

}  // namespace activity
