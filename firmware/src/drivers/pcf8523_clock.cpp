#include "drivers/pcf8523_clock.h"

namespace drivers {

bool Pcf8523Clock::begin() {
  ok_ = rtc_.begin(&wire_);
  if (!ok_) {
    return false;
  }
  if (!rtc_.initialized() || rtc_.lostPower()) {
    rtc_.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  rtc_.start();
  return true;
}

bool Pcf8523Clock::now(hal::ClockTime& out) {
  if (!ok_) {
    return false;
  }
  const DateTime t = rtc_.now();
  if (!t.isValid()) {
    return false;
  }
  out.year = t.year();
  out.month = t.month();
  out.day = t.day();
  out.hour = t.hour();
  out.minute = t.minute();
  out.second = t.second();
  return true;
}

}  // namespace drivers
