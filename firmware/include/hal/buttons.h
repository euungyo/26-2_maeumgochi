#pragma once

#include <stdint.h>

namespace hal {

enum class ButtonId : uint8_t { kLeft = 0, kCenter = 1, kRight = 2 };
constexpr uint8_t kButtonCount = 3;

// 버튼의 단순 눌림 상태만 제공합니다.
// 짧게/길게 누름 판정은 ui 계층의 로직이 담당합니다.
class Buttons {
 public:
  virtual ~Buttons() = default;

  virtual bool begin() = 0;
  // 입력을 읽어 갱신합니다. 메인 루프에서 자주 호출합니다.
  virtual void update() = 0;
  virtual bool isPressed(ButtonId id) const = 0;
};

}  // namespace hal