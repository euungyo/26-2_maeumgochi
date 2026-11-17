#pragma once

#include <stdint.h>

#include "hal/haptic.h"

namespace drivers {

// MOSFET 게이트 핀(HIGH = 진동)으로 진동모터를 켭니다.
// vibrate() 후 메인 루프에서 update()를 계속 호출해야 지정한 시간에 꺼집니다.
class GpioHaptic : public hal::Haptic {
 public:
  // 모터가 멈추지 않는 사고를 막기 위한 최대 진동 시간
  static constexpr uint16_t kMaxDurationMs = 2000;

  explicit GpioHaptic(int gatePin);

  bool begin() override;
  void vibrate(uint16_t durationMs) override;
  void stop() override;
  void update() override;

 private:
  int pin_;
  bool on_ = false;
  uint32_t startedMs_ = 0;
  uint16_t durationMs_ = 0;
};

}  // namespace drivers
