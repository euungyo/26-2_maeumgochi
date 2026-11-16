#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <stdint.h>

#include "hal/display.h"

namespace drivers {

struct St7789Config {
  int sck = -1;
  int mosi = -1;
  int rst = -1;
  int dc = -1;
  int cs = -1;  // -1이면 CS를 GND에 고정한 배선입니다.
  uint8_t spiMode = SPI_MODE2;
  uint8_t rotation = 0;
};

// 240x240 ST7789. 모든 그리기는 메모리 캔버스에 쌓이고 present()에서 LCD로 전송됩니다.
// 백라이트(BLK)는 3V3에 고정되어 있어 setBacklight()는 아무 일도 하지 않습니다.
class St7789Display : public hal::Display {
 public:
  static constexpr int16_t kSize = 240;

  explicit St7789Display(const St7789Config& config);
  ~St7789Display() override;

  // 캔버스(약 115KB)를 할당하지 못하거나 SPI 초기화에 실패하면 false를 반환합니다.
  bool begin() override;
  int16_t width() const override { return kSize; }
  int16_t height() const override { return kSize; }

  void clear(uint16_t color) override;
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  void drawText(int16_t x, int16_t y, const char* text, uint16_t color, uint8_t scale = 1) override;
  void drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* pixels) override;
  void setBacklight(uint8_t level) override { (void)level; }

  // UI 렌더러가 그릴 대상입니다. begin() 성공 후에만 호출합니다.
  Adafruit_GFX& canvas() { return *canvas_; }
  // 캔버스 내용을 LCD로 전송합니다.
  void present();
  bool isReady() const { return ready_; }

 private:
  St7789Config config_;
  Adafruit_ST7789 tft_;
  GFXcanvas16* canvas_ = nullptr;
  bool ready_ = false;
};

}  // namespace drivers