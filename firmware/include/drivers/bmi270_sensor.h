#pragma once

#include <Wire.h>
#include <stdint.h>

#include "hal/motion_sensor.h"

namespace drivers {

struct Bmi270Config {
  // PCF8523 RTC(0x68)와 겹치지 않도록 주소 점퍼를 바꿔 0x69로 사용합니다. (hardware/wiring/pin-map.md)
  uint8_t address = 0x69;
  // 센서가 없거나 끊겼을 때 다시 연결을 시도하는 간격
  uint32_t reconnectIntervalMs = 2000;
};

// BMI270을 I2C로 읽어 hal::MotionSensor로 제공합니다.
// 걸음 수는 BMI270 내장 걸음 수 카운터 값을 그대로 전달합니다.
// Wire.begin()은 호출자가 먼저 해 둡니다. 같은 I2C 버스를 다른 센서와 함께 씁니다.
class Bmi270Sensor : public hal::MotionSensor {
 public:
  explicit Bmi270Sensor(TwoWire& wire = Wire, Bmi270Config config = Bmi270Config());

  bool begin() override;
  // 루프에서 주기적으로 호출합니다. 끊긴 상태면 reconnectIntervalMs마다 다시 연결을 시도합니다.
  void update() override;
  bool isValid() const override { return reading_.valid; }
  hal::MotionReading latest() const override { return reading_; }

 private:
  bool connect();

  TwoWire& wire_;
  Bmi270Config config_;
  hal::MotionReading reading_ = {};
  bool connected_ = false;
  uint32_t lastConnectAttemptMs_ = 0;
};

}  // namespace drivers
