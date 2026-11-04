#include <Arduino.h>
#include <RTClib.h>
#include <Wire.h>

namespace {
RTC_PCF8523 rtc;

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
  Serial.println("=== PCF8523 RTC diagnostic ===");
  Serial.printf("I2C address=0x68, SDA=GPIO%d, SCL=GPIO%d\n", SDA, SCL);

  if (!rtc.begin(&Wire)) {
    Serial.println("ERROR: PCF8523 not found at 0x68. Check power and I2C wiring.");
    while (true) {
      delay(1000);
    }
  }

  if (!rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC was not initialized; setting it to this firmware's build time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc.start();
  Serial.println("PCF8523 connected. Remove main power later to verify the CR1220 backup.");
}

void loop() {
  const DateTime now = rtc.now();
  Serial.printf("%04d-%02d-%02d %02d:%02d:%02d\n", now.year(), now.month(), now.day(),
                now.hour(), now.minute(), now.second());
  delay(1000);
}
