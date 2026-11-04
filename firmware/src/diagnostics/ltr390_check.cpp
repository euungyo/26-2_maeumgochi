#include <Adafruit_LTR390.h>
#include <Arduino.h>
#include <Wire.h>

namespace {
Adafruit_LTR390 sensor;

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
  Serial.println("=== LTR390 diagnostic ===");
  Serial.printf("I2C address=0x53, SDA=GPIO%d, SCL=GPIO%d\n", SDA, SCL);

  if (!sensor.begin(&Wire)) {
    Serial.println("ERROR: LTR390 not found at 0x53. Check power and I2C wiring.");
    while (true) {
      delay(1000);
    }
  }

  sensor.setGain(LTR390_GAIN_3);
  sensor.setResolution(LTR390_RESOLUTION_16BIT);
  Serial.println("LTR390 connected. Cover it and expose it to light to compare readings.");
}

void loop() {
  sensor.setMode(LTR390_MODE_ALS);
  delay(150);
  const uint32_t ambient = sensor.readALS();

  sensor.setMode(LTR390_MODE_UVS);
  delay(150);
  const uint32_t ultraviolet = sensor.readUVS();

  Serial.printf("ambient_raw=%lu uv_raw=%lu\n", ambient, ultraviolet);
  delay(700);
}
