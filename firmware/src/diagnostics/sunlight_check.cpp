// LTR390 드라이버와 SunlightTracker를 보드에서 함께 확인합니다.
// 야외/실내/창가에서 lux와 uv 값을 읽어 햇빛 기준(SunlightConfig)을 실측으로 정할 때 씁니다.
// 실행:
//   pio run -e sunlight_check -t upload
//   pio device monitor -e sunlight_check
// 시리얼 모니터에서 r을 입력하면 누적 시간을 0으로 되돌립니다.
#include <Arduino.h>
#include <Wire.h>

#include "activity/sunlight_tracker.h"
#include "drivers/ltr390_sensor.h"

namespace {
constexpr uint32_t kPrintIntervalMs = 1000;

drivers::Ltr390Sensor sensor;
activity::SunlightTracker tracker;
uint32_t lastPrintMs = 0;

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
  Serial.println("=== Sunlight check (LTR390 + SunlightTracker) ===");
  Serial.println("Type r + Enter to reset the accumulated time.");
  if (!sensor.begin()) {
    Serial.println("WARNING: LTR390 not found at 0x53. Retrying in the background.");
  }
}

void loop() {
  const uint32_t now = millis();

  sensor.update();
  tracker.update(sensor.latest(), now);

  while (Serial.available() > 0) {
    const int c = Serial.read();
    if (c == 'r' || c == 'R') {
      tracker.reset();
      Serial.println("[reset] sunlight time restarted from 0");
    }
  }

  if (now - lastPrintMs >= kPrintIntervalMs) {
    lastPrintMs = now;
    const hal::LightReading r = sensor.latest();
    Serial.printf("lux=%.0f uvi=%.2f sensor=%s sunny=%s sunlight=%lus\n", r.lux, r.uvIndex,
                  r.valid ? "ok" : "MISSING", tracker.isSunny() ? "yes" : "no",
                  static_cast<unsigned long>(tracker.seconds()));
  }
  delay(5);
}
