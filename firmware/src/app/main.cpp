#include <Adafruit_ST7789.h>
#include <Arduino.h>
#include <SPI.h>

#include "ui/button_gesture.h"
#include "ui/screen_navigator.h"
#include "ui/screen_renderer.h"

namespace {
// 핀 문서 기준 GPIO 번호 (D8=7, D10=9, D2=3, D3=4, D1=2 / D0=1, D9=8, D11=38)
constexpr int kLcdSck = 7, kLcdMosi = 9, kLcdCs = 3, kLcdDc = 4, kLcdRst = 2;
constexpr int kButtonPins[3] = {1, 8, 38};  // 왼쪽, 가운데, 오른쪽

Adafruit_ST7789 tft(&SPI, kLcdCs, kLcdDc, kLcdRst);
GFXcanvas16 canvas(240, 240);  // 약 115KB, 깜빡임 방지용
ui::ScreenNavigator nav;
ui::ButtonGesture gestures[3];

void present() {
  ui::renderScreen(canvas, nav);
  tft.drawRGBBitmap(0, 0, canvas.getBuffer(), 240, 240);
}
}  // namespace

void setup() {
  Serial.begin(115200);
  SPI.begin(kLcdSck, -1, kLcdMosi, kLcdCs);
  tft.init(240, 240);
  tft.setRotation(0);
  for (int pin : kButtonPins) pinMode(pin, INPUT_PULLUP);
  nav.completeCheckIn();  // 데모는 메인 화면에서 시작
  present();
}

void loop() {
  const uint32_t now = millis();
  bool changed = false;
  for (int i = 0; i < 3; ++i) {
    const ui::GestureEvent ev = gestures[i].update(digitalRead(kButtonPins[i]) == LOW, now);
    if (ev == ui::GestureEvent::kShortPress) {
      nav.handle(i == 0 ? ui::NavInput::kLeftShort
                : i == 1 ? ui::NavInput::kCenterShort
                         : ui::NavInput::kRightShort);
      changed = true;
    } else if (ev == ui::GestureEvent::kLongPress && i == 1) {
      nav.handle(ui::NavInput::kCenterLong);
      changed = true;
    }
  }
  if (changed) present();
  delay(5);
}