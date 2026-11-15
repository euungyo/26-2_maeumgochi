// BMI270 드라이버와 StepTracker를 보드에서 함께 확인하는 진단 프로그램입니다.
// 실행:
//   pio run -e step_tracker_check -t upload
//   pio device monitor -e step_tracker_check
// 시리얼 모니터에서 r을 입력하면 걸음 수를 0부터 다시 셉니다(미션 시작과 같음).
#include <Arduino.h>
#include <Wire.h>

#include "activity/step_tracker.h"
#include "drivers/bmi270_sensor.h"

namespace {
constexpr uint32_t kSampleIntervalMs = 100;
constexpr uint32_t kPrintIntervalMs = 1000;

drivers::Bmi270Sensor sensor;
activity::StepTracker tracker;
uint32_t lastSampleMs = 0;
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
  Serial.println("=== Step tracker check (BMI270 + StepTracker) ===");
  Serial.println("Type r + Enter to reset the step count.");
  if (!sensor.begin()) {
    Serial.println("WARNING: BMI270 not found at 0x69. Retrying in the background.");
  }
}

void loop() {
  const uint32_t now = millis();

  if (now - lastSampleMs >= kSampleIntervalMs) {
    lastSampleMs = now;
    sensor.update();
    tracker.update(sensor.latest(), now);
  }

  while (Serial.available() > 0) {
    const int c = Serial.read();
    if (c == 'r' || c == 'R') {
      tracker.reset();
      Serial.println("[reset] step count restarted from 0");
    }
  }

  if (now - lastPrintMs >= kPrintIntervalMs) {
    lastPrintMs = now;
    const hal::MotionReading r = sensor.latest();
    Serial.printf("steps=%lu active=%lus walking=%s sensor=%s raw=%lu | accel[g] % .2f % .2f % .2f\n",
                  static_cast<unsigned long>(tracker.steps()),
                  static_cast<unsigned long>(tracker.activeSeconds()),
                  tracker.isWalking() ? "yes" : "no", r.valid ? "ok" : "MISSING",
                  static_cast<unsigned long>(r.stepCount), r.accelX, r.accelY, r.accelZ);
  }
}
