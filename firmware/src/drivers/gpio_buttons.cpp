#include "drivers/gpio_buttons.h"

#include <Arduino.h>

#include "drivers/board_pins.h"

namespace {
constexpr int kPins[hal::kButtonCount] = {board::kButtonLeft, board::kButtonCenter,
                                          board::kButtonRight};
}  // namespace

bool GpioButtons::begin() {
  for (uint8_t i = 0; i < hal::kButtonCount; ++i) {
    pinMode(kPins[i], INPUT_PULLUP);
  }
  return true;
}

void GpioButtons::update() {
  for (uint8_t i = 0; i < hal::kButtonCount; ++i) {
    pressed_[i] = digitalRead(kPins[i]) == LOW;
  }
}

bool GpioButtons::isPressed(hal::ButtonId id) const { return pressed_[static_cast<uint8_t>(id)]; }