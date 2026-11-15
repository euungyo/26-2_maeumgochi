#pragma once

#include "hal/buttons.h"

namespace drivers {

// 풀업 입력을 읽습니다. 버튼을 누르면 핀이 GND와 연결되어 LOW가 됩니다.
class GpioButtons : public hal::Buttons {
 public:
  GpioButtons(int leftPin, int centerPin, int rightPin);

  bool begin() override;
  void update() override;
  bool isPressed(hal::ButtonId id) const override;

 private:
  int pins_[hal::kButtonCount];
  bool pressed_[hal::kButtonCount] = {};
};

}  // namespace drivers