#include "activity/sunlight_tracker.h"

namespace activity {

SunlightTracker::SunlightTracker(SunlightConfig config) : config_(config) {}

void SunlightTracker::reset() {
  sunMs_ = 0;
  hasLast_ = false;
  sunny_ = false;
}

void SunlightTracker::restore(uint32_t seconds) {
  sunMs_ = seconds * 1000u;
  hasLast_ = false;  // 복원 직전의 공백 시간은 세지 않습니다.
  sunny_ = false;
}

bool SunlightTracker::meetsThreshold(const hal::LightReading& reading) const {
  if (!reading.valid) {
    return false;
  }
  if (reading.lux >= config_.luxThreshold) {
    return true;
  }
  return config_.uvIndexThreshold > 0.0f && reading.uvIndex >= config_.uvIndexThreshold;
}

void SunlightTracker::update(const hal::LightReading& reading, uint32_t nowMs) {
  sensorValid_ = reading.valid;

  // 직전 측정이 햇빛이었다면 그때부터 지금까지를 햇빛 시간으로 더합니다.
  // uint32_t 뺄셈이라 millis()가 한 바퀴 돌아도 간격이 올바르게 계산됩니다.
  if (hasLast_) {
    const uint32_t elapsed = nowMs - lastMs_;
    if (sunny_ && elapsed <= config_.maxGapMs) {
      sunMs_ += elapsed;
    }
  }

  hasLast_ = true;
  lastMs_ = nowMs;
  sunny_ = meetsThreshold(reading);
}

}  // namespace activity
