#pragma once

#include <stdint.h>

namespace hal {

// vibrate() 후 메인 루프에서 update()를 계속 호출합니다.
class Haptic {
 public:
  virtual ~Haptic() = default;

  virtual bool begin() = 0;
  virtual void vibrate(uint16_t durationMs) = 0;
  virtual void stop() = 0;
  // 지정한 시간이 지나면 모터를 끕니다.
  virtual void update() = 0;
};

}  // namespace hal