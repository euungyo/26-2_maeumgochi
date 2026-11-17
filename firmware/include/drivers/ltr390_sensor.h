#pragma once

#include <Wire.h>
#include <stdint.h>

#include "hal/light_sensor.h"

namespace drivers {

struct Ltr390Config {
  // 한 번 측정하는 주기. 조도와 UV를 번갈아 읽으므로 한 번에 약 300ms가 걸립니다.
  uint32_t measureIntervalMs = 1000;
  // 모드를 바꾼 뒤 변환이 끝나기를 기다리는 시간 (진단 프로그램과 같은 값)
  uint32_t conversionWaitMs = 150;
  // 센서가 없거나 끊겼을 때 다시 연결을 시도하는 간격
  uint32_t reconnectIntervalMs = 2000;
};

// LTR390을 I2C(0x53)로 읽어 hal::LightSensor로 제공합니다.
// delay() 없이 상태 기계로 동작하므로 update()를 루프마다 불러도 버튼 입력이 막히지 않습니다.
// 16비트 해상도에서 최대 65535 카운트라서 한낮 직사광선에서는 조도가 약 52,000 lux에서 포화됩니다.
// Wire.begin()은 호출자가 먼저 해 둡니다. 같은 I2C 버스를 다른 센서와 함께 씁니다.
class Ltr390Sensor : public hal::LightSensor {
 public:
  explicit Ltr390Sensor(TwoWire& wire = Wire, Ltr390Config config = Ltr390Config());

  bool begin() override;
  void update() override;
  bool isValid() const override { return reading_.valid; }
  hal::LightReading latest() const override { return reading_; }

 private:
  enum class State : uint8_t { kDisconnected, kIdle, kWaitAls, kWaitUvs };

  bool connect();
  bool isPresent();

  TwoWire& wire_;
  Ltr390Config config_;
  hal::LightReading reading_ = {0.0f, 0.0f, false};
  State state_ = State::kDisconnected;
  uint32_t lastAttemptMs_ = 0;
  uint32_t lastCycleMs_ = 0;
  uint32_t phaseStartMs_ = 0;
  float lux_ = 0.0f;
};

}  // namespace drivers
