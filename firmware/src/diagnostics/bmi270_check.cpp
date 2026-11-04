#include <Arduino.h>
#include <SparkFun_BMI270_Arduino_Library.h>
#include <Wire.h>

namespace {
constexpr uint8_t kBmi270Address = BMI2_I2C_SEC_ADDR;  // 0x69: PCF8523(0x68)과 충돌 방지
BMI270 imu;

void waitForSerial(unsigned long timeoutMs) {
  const unsigned long startedAt = millis();
  while (!Serial && millis() - startedAt < timeoutMs) {
    delay(10);
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  waitForSerial(2000);
  Wire.begin(SDA, SCL);

  Serial.println();
  Serial.println("=== BMI270 diagnostic ===");
  Serial.printf("I2C address=0x%02X, SDA=GPIO%d, SCL=GPIO%d\n", kBmi270Address, SDA, SCL);

  if (imu.beginI2C(kBmi270Address, Wire) != BMI2_OK) {
    Serial.println("ERROR: BMI270 not found at 0x69.");
    Serial.println("Check power, Qwiic wiring, and the address jumper.");
    while (true) {
      delay(1000);
    }
  }

  Serial.println("BMI270 connected. Move the board to verify changing values.");
}

void loop() {
  if (imu.getSensorData() != BMI2_OK) {
    Serial.println("ERROR: failed to read BMI270");
    delay(1000);
    return;
  }

  Serial.printf("accel[g] x=% .3f y=% .3f z=% .3f | gyro[dps] x=% .3f y=% .3f z=% .3f\n",
                imu.data.accelX, imu.data.accelY, imu.data.accelZ, imu.data.gyroX,
                imu.data.gyroY, imu.data.gyroZ);
  delay(200);
}
