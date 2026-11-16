#include "drivers/gpio_buttons.h"

#include <Arduino.h>

namespace drivers {

GpioButtons::GpioButtons(int leftPin, int centerPin, int rightPin)
    : pins_{leftPin, centerPin, rightPin} {}

bool GpioButtons::begin() {
  for (uint8_t i = 0; i < hal::kButtonCount; ++i) {
    pinMode(pins_[i], INPUT_PULLUP);
    pressed_[i] = false;
  }
  return true;
}

void GpioButtons::update() {
  for (uint8_t i = 0; i < hal::kButtonCount; ++i) {
    pressed_[i] = digitalRead(pins_[i]) == LOW;
  }
}

bool GpioButtons::isPressed(hal::ButtonId id) const { return pressed_[static_cast<uint8_t>(id)]; }

}  // namespace drivers