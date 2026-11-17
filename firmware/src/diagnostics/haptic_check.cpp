// 진동모터와 MOSFET 회로를 보드에서 확인합니다. 2초마다 200ms 진동, 5번째마다 600ms 진동합니다.
#include <Arduino.h>

#include "drivers/board_pins.h"
#include "drivers/gpio_haptic.h"

namespace {
drivers::GpioHaptic haptic(board::kHapticGate);
uint32_t lastPulseMs = 0;
int pulseCount = 0;

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
  haptic.begin();
  Serial.println();
  Serial.println("=== Haptic check ===");
  Serial.printf("gate=GPIO%d (HIGH = vibrate)\n", board::kHapticGate);
}

void loop() {
  const uint32_t now = millis();
  if (now - lastPulseMs >= 2000) {
    lastPulseMs = now;
    ++pulseCount;
    const uint16_t duration = (pulseCount % 5 == 0) ? 600 : 200;
    Serial.printf("pulse #%d: %ums\n", pulseCount, duration);
    haptic.vibrate(duration);
  }
  haptic.update();
  delay(5);
}
