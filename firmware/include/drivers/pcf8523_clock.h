#pragma once

#include <RTClib.h>
#include <Wire.h>

#include "hal/clock.h"

namespace drivers {

// PCF8523 RTC(0x68)를 hal::Clock으로 제공합니다.
// RTC가 초기화되지 않았거나 전원이 한 번 끊겼다면 이 펌웨어의 빌드 시각으로 맞춥니다.
// (pcf8523_check 진단과 같은 방식입니다. 배터리를 뺐다 끼우면 시각이 빌드 시점으로 돌아갑니다.)
// Wire.begin()은 호출자가 먼저 해 둡니다.
class Pcf8523Clock : public hal::Clock {
 public:
  explicit Pcf8523Clock(TwoWire& wire = Wire) : wire_(wire) {}

  bool begin() override;
  bool now(hal::ClockTime& out) override;

 private:
  TwoWire& wire_;
  RTC_PCF8523 rtc_;
  bool ok_ = false;
};

}  // namespace drivers
