#include "drivers/ltr390_sensor.h"

#include <Adafruit_LTR390.h>
#include <Arduino.h>

#include "activity/ltr390_math.h"

namespace drivers {
namespace {
// Adafruit 라이브러리 객체는 헤더에 노출하지 않습니다. 보드에 LTR390은 하나뿐입니다.
Adafruit_LTR390 ltr;
constexpr uint8_t kAddress = 0x53;
}  // namespace

Ltr390Sensor::Ltr390Sensor(TwoWire& wire, Ltr390Config config) : wire_(wire), config_(config) {}

bool Ltr390Sensor::begin() {
  lastAttemptMs_ = millis();
  return connect();
}

bool Ltr390Sensor::connect() {
  reading_ = {0.0f, 0.0f, false};
  state_ = State::kDisconnected;

  if (!ltr.begin(&wire_)) {
    return false;
  }
  // activity/ltr390_math.h의 kLtr390Gain, kLtr390IntegrationMs와 같아야 합니다.
  ltr.setGain(LTR390_GAIN_3);
  ltr.setResolution(LTR390_RESOLUTION_16BIT);

  state_ = State::kIdle;
  lastCycleMs_ = millis() - config_.measureIntervalMs;  // 첫 측정을 바로 시작합니다.
  return true;
}

bool Ltr390Sensor::isPresent() {
  wire_.beginTransmission(kAddress);
  return wire_.endTransmission() == 0;
}

void Ltr390Sensor::update() {
  const uint32_t nowMs = millis();

  switch (state_) {
    case State::kDisconnected:
      if (nowMs - lastAttemptMs_ < config_.reconnectIntervalMs) {
        return;
      }
      lastAttemptMs_ = nowMs;
      connect();
      return;

    case State::kIdle:
      if (nowMs - lastCycleMs_ < config_.measureIntervalMs) {
        return;
      }
      lastCycleMs_ = nowMs;
      if (!isPresent()) {
        // 센서가 빠졌습니다. 값을 무효로 하고 재연결을 시도합니다.
        reading_.valid = false;
        state_ = State::kDisconnected;
        lastAttemptMs_ = nowMs;
        return;
      }
      ltr.setMode(LTR390_MODE_ALS);
      phaseStartMs_ = nowMs;
      state_ = State::kWaitAls;
      return;

    case State::kWaitAls:
      if (nowMs - phaseStartMs_ < config_.conversionWaitMs) {
        return;
      }
      lux_ = activity::ltr390Lux(ltr.readALS());
      ltr.setMode(LTR390_MODE_UVS);
      phaseStartMs_ = nowMs;
      state_ = State::kWaitUvs;
      return;

    case State::kWaitUvs:
      if (nowMs - phaseStartMs_ < config_.conversionWaitMs) {
        return;
      }
      reading_ = {lux_, activity::ltr390Uvi(ltr.readUVS()), true};
      state_ = State::kIdle;
      return;
  }
}

}  // namespace drivers
