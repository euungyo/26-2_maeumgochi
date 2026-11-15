#include "drivers/bmi270_sensor.h"

#include <Arduino.h>
#include <SparkFun_BMI270_Arduino_Library.h>

namespace drivers {
namespace {
// SparkFun 라이브러리 객체는 헤더에 노출하지 않습니다.
// 보드에 BMI270은 하나뿐이라 파일 안에 하나만 둡니다.
BMI270 imu;
}  // namespace

Bmi270Sensor::Bmi270Sensor(TwoWire& wire, Bmi270Config config) : wire_(wire), config_(config) {}

bool Bmi270Sensor::begin() {
  lastConnectAttemptMs_ = millis();
  return connect();
}

bool Bmi270Sensor::connect() {
  connected_ = false;
  reading_.valid = false;

  if (imu.beginI2C(config_.address, wire_) != BMI2_OK) {
    return false;
  }
  // 내장 걸음 수 카운터를 켭니다. 센서 전원이 다시 들어오면 카운터는 0부터 셉니다.
  if (imu.enableFeature(BMI2_STEP_COUNTER) != BMI2_OK) {
    return false;
  }

  connected_ = true;
  return true;
}

void Bmi270Sensor::update() {
  const uint32_t nowMs = millis();

  if (!connected_) {
    if (nowMs - lastConnectAttemptMs_ < config_.reconnectIntervalMs) {
      return;
    }
    lastConnectAttemptMs_ = nowMs;
    if (!connect()) {
      return;
    }
  }

  uint32_t stepCount = 0;
  if (imu.getSensorData() != BMI2_OK || imu.getStepCount(&stepCount) != BMI2_OK) {
    // 읽기에 실패하면 끊긴 것으로 보고 다음 update()부터 재연결을 시도합니다.
    connected_ = false;
    reading_.valid = false;
    lastConnectAttemptMs_ = nowMs;
    return;
  }

  reading_.accelX = imu.data.accelX;
  reading_.accelY = imu.data.accelY;
  reading_.accelZ = imu.data.accelZ;
  reading_.gyroX = imu.data.gyroX;
  reading_.gyroY = imu.data.gyroY;
  reading_.gyroZ = imu.data.gyroZ;
  reading_.stepCount = stepCount;
  reading_.valid = true;
}

}  // namespace drivers
