// 버튼 3개의 배선과 짧게/길게 누름 판정을 보드에서 확인합니다.
#include <Arduino.h>

#include "drivers/board_pins.h"
#include "drivers/gpio_buttons.h"
#include "ui/nav_input_mapper.h"

namespace {
drivers::GpioButtons buttons(board::kButtonLeft, board::kButtonCenter, board::kButtonRight);
ui::NavInputMapper mapper;

void waitForSerial(unsigned long timeoutMs) {
  const unsigned long startedAt = millis();
  while (!Serial && millis() - startedAt < timeoutMs) {
    delay(10);
  }
}

const char* name(ui::NavInput input) {
  switch (input) {
    case ui::NavInput::kLeftShort:
      return "LEFT short";
    case ui::NavInput::kCenterShort:
      return "CENTER short";
    case ui::NavInput::kRightShort:
      return "RIGHT short";
    case ui::NavInput::kCenterLong:
      return "CENTER long (back)";
  }
  return "?";
}
}  // namespace

void setup() {
  Serial.begin(115200);
  waitForSerial(2000);
  buttons.begin();
  Serial.println();
  Serial.println("=== Button check ===");
  Serial.printf("L=GPIO%d C=GPIO%d R=GPIO%d (INPUT_PULLUP)\n", board::kButtonLeft,
                board::kButtonCenter, board::kButtonRight);
}

void loop() {
  buttons.update();
  const bool pressed[hal::kButtonCount] = {buttons.isPressed(hal::ButtonId::kLeft),
                                           buttons.isPressed(hal::ButtonId::kCenter),
                                           buttons.isPressed(hal::ButtonId::kRight)};

  ui::NavInput input;
  if (mapper.update(pressed, millis(), input)) {
    Serial.println(name(input));
  }
  delay(5);
}