#include "drivers/gpio_haptic.h"

#include <Arduino.h>

namespace drivers {

GpioHaptic::GpioHaptic(int gatePin) : pin_(gatePin) {}

bool GpioHaptic::begin() {
  pinMode(pin_, OUTPUT);
  digitalWrite(pin_, LOW);
  on_ = false;
  return true;
}

void GpioHaptic::vibrate(uint16_t durationMs) {
  if (durationMs == 0) {
    stop();
    return;
  }
  durationMs_ = durationMs > kMaxDurationMs ? kMaxDurationMs : durationMs;
  startedMs_ = millis();
  on_ = true;
  digitalWrite(pin_, HIGH);
}

void GpioHaptic::stop() {
  digitalWrite(pin_, LOW);
  on_ = false;
}

void GpioHaptic::update() {
  if (on_ && millis() - startedMs_ >= durationMs_) {
    stop();
  }
}

}  // namespace drivers
