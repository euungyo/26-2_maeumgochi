#pragma once

#include "hal/buttons.h"

// 버튼 3개를 INPUT_PULLUP으로 읽습니다. 버튼의 반대쪽은 GND에 연결합니다.
class GpioButtons : public hal::Buttons {
 public:
  bool begin() override;
  void update() override;
  bool isPressed(hal::ButtonId id) const override;

 private:
  bool pressed_[hal::kButtonCount] = {};
};