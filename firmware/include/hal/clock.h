#pragma once

#include <stdint.h>

namespace hal {

struct ClockTime {
  uint16_t year;
  uint8_t month;   // 1~12
  uint8_t day;     // 1~31
  uint8_t hour;    // 0~23
  uint8_t minute;  // 0~59
  uint8_t second;  // 0~59
};

// 날짜와 시각의 출처입니다. 실제 장치는 PCF8523 RTC, PC 시험은 가짜 시계입니다.
class Clock {
 public:
  virtual ~Clock() = default;

  // 시계를 찾지 못하면 false를 반환합니다. 이 경우에도 앱은 계속 동작해야 합니다.
  virtual bool begin() = 0;
  // 시각을 읽을 수 없으면 false를 반환하고 out은 건드리지 않습니다.
  virtual bool now(ClockTime& out) = 0;
};

}  // namespace hal
