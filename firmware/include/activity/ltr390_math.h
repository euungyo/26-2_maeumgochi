#pragma once

#include <stdint.h>

namespace activity {

// 아래 설정은 drivers/ltr390_sensor.cpp의 LTR390_GAIN_3, LTR390_RESOLUTION_16BIT와 같아야 합니다.
constexpr float kLtr390Gain = 3.0f;
constexpr float kLtr390IntegrationMs = 25.0f;  // 16비트 변환 시간
constexpr float kLtr390WindowFactor = 1.0f;    // 센서 앞에 투명 덮개를 달면 데이터시트 표로 보정

// 데이터시트 공식: lux = 0.6 * ALS / (Gain * Int) * WFAC, Int = 변환시간 / 100ms
inline float ltr390Lux(uint32_t rawAls) {
  return 0.6f * static_cast<float>(rawAls) / (kLtr390Gain * (kLtr390IntegrationMs / 100.0f)) *
         kLtr390WindowFactor;
}

// 데이터시트 공식: UVI = UVS / (2300 * Gain/18 * 변환시간/400ms) * WFAC
inline float ltr390Uvi(uint32_t rawUvs) {
  return static_cast<float>(rawUvs) /
         (2300.0f * (kLtr390Gain / 18.0f) * (kLtr390IntegrationMs / 400.0f)) * kLtr390WindowFactor;
}

}  // namespace activity
