#pragma once

#include <stdint.h>

namespace hal {

// RGB565 색상
constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) {
  return static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

constexpr uint16_t kColorBlack = 0x0000;
constexpr uint16_t kColorWhite = 0xFFFF;

class Display {
 public:
  virtual ~Display() = default;

  virtual bool begin() = 0;
  virtual int16_t width() const = 0;
  virtual int16_t height() const = 0;

  virtual void clear(uint16_t color) = 0;
  virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
  virtual void drawText(int16_t x, int16_t y, const char* text, uint16_t color,
                        uint8_t scale = 1) = 0;
  // pixels는 w * h 개의 RGB565 값입니다.
  virtual void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h,
                          const uint16_t* pixels) = 0;

  // 0(꺼짐) ~ 255(최대)
  virtual void setBacklight(uint8_t level) = 0;
};

}  // namespace hal