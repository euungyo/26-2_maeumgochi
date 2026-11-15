#include "drivers/st7789_display.h"

namespace drivers {

St7789Display::St7789Display(const St7789Config& config)
    : config_(config),
      tft_(static_cast<int8_t>(config.cs), static_cast<int8_t>(config.dc),
           static_cast<int8_t>(config.rst)) {}

St7789Display::~St7789Display() { delete canvas_; }

bool St7789Display::begin() {
  if (canvas_ == nullptr) {
    // 전역 생성 시점이 아니라 begin()에서 할당해 PSRAM 초기화 순서 문제를 피합니다.
    canvas_ = new GFXcanvas16(kSize, kSize);
  }
  if (canvas_ == nullptr || canvas_->getBuffer() == nullptr) {
    return false;
  }

  // MISO를 -1로 둡니다. 기본 SPI.begin()은 D9(오른쪽 버튼)를 MISO로 잡습니다.
  SPI.begin(config_.sck, -1, config_.mosi, -1);
  tft_.init(kSize, kSize, config_.spiMode);
  tft_.setRotation(config_.rotation);

  canvas_->fillScreen(0);
  ready_ = true;
  present();
  return true;
}

void St7789Display::clear(uint16_t color) {
  if (ready_) canvas_->fillScreen(color);
}

void St7789Display::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (ready_) canvas_->fillRect(x, y, w, h, color);
}

void St7789Display::drawText(int16_t x, int16_t y, const char* text, uint16_t color,
                             uint8_t scale) {
  if (!ready_) return;
  canvas_->setTextColor(color);
  canvas_->setTextSize(scale);
  canvas_->setCursor(x, y);
  canvas_->print(text);
}

void St7789Display::drawBitmap(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t* pixels) {
  if (ready_) canvas_->drawRGBBitmap(x, y, pixels, w, h);
}

void St7789Display::present() {
  if (!ready_) return;
  tft_.drawRGBBitmap(0, 0, canvas_->getBuffer(), kSize, kSize);
}

}  // namespace drivers