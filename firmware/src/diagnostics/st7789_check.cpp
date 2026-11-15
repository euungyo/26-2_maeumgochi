// ST7789 배선, 색 순서, 화면 렌더링 시간을 보드에서 확인합니다.
#include <Arduino.h>

#include "drivers/board_pins.h"
#include "drivers/st7789_display.h"
#include "hal/display.h"
#include "ui/screen_navigator.h"
#include "ui/screen_renderer.h"

namespace {

drivers::St7789Config makeConfig() {
  drivers::St7789Config config;
  config.sck = board::kLcdSck;
  config.mosi = board::kLcdMosi;
  config.rst = board::kLcdRes;
  config.dc = board::kLcdDc;
  config.cs = board::kLcdCs;
  return config;
}

drivers::St7789Display display(makeConfig());
ui::ScreenNavigator nav;
int step = -1;  // -1: 체크인, 0: 메인, 1~: 아래 입력 순서

// 모든 화면이 한 번씩 나오도록 이동합니다.
const ui::NavInput kSteps[] = {
    ui::NavInput::kLeftShort,    // 생활 메뉴
    ui::NavInput::kCenterShort,  // 미션 목록
    ui::NavInput::kCenterLong,   // 생활 메뉴
    ui::NavInput::kCenterLong,   // 메인
    ui::NavInput::kCenterShort,  // 미션 상세
    ui::NavInput::kCenterLong,   // 메인
    ui::NavInput::kRightShort,   // 캐릭터 메뉴
    ui::NavInput::kCenterShort,  // 성장
    ui::NavInput::kCenterLong,   // 캐릭터 메뉴
    ui::NavInput::kRightShort,   // 상점 선택
    ui::NavInput::kCenterShort,  // 상점
    ui::NavInput::kCenterLong,   // 캐릭터 메뉴
    ui::NavInput::kRightShort,
    ui::NavInput::kRightShort,   // 가방 선택
    ui::NavInput::kCenterShort,  // 가방
    ui::NavInput::kCenterLong,   // 캐릭터 메뉴
    ui::NavInput::kCenterLong,   // 메인
};
constexpr int kStepCount = sizeof(kSteps) / sizeof(kSteps[0]);

void waitForSerial(unsigned long timeoutMs) {
  const unsigned long startedAt = millis();
  while (!Serial && millis() - startedAt < timeoutMs) {
    delay(10);
  }
}

void colorBars() {
  const uint16_t colors[] = {hal::rgb565(255, 0, 0), hal::rgb565(0, 255, 0), hal::rgb565(0, 0, 255),
                             hal::kColorWhite, hal::kColorBlack};
  const char* names[] = {"RED", "GREEN", "BLUE", "WHITE", "BLACK"};
  for (int i = 0; i < 5; ++i) {
    display.clear(colors[i]);
    display.present();
    Serial.printf("color bar: %s (실제 색과 다르면 색 순서/반전 설정 확인)\n", names[i]);
    delay(1000);
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  waitForSerial(2000);

  Serial.println();
  Serial.println("=== ST7789 diagnostic ===");
  Serial.printf("SCK=GPIO%d MOSI=GPIO%d RES=GPIO%d DC=GPIO%d CS=%s\n", board::kLcdSck,
                board::kLcdMosi, board::kLcdRes, board::kLcdDc,
                board::kLcdCs < 0 ? "GND fixed" : "pin");

  if (!display.begin()) {
    Serial.println("ERROR: display.begin() failed (canvas allocation or SPI init)");
    while (true) {
      delay(1000);
    }
  }
  Serial.println("display ready");
  colorBars();
}

void loop() {
  if (step == -1) {
    nav = ui::ScreenNavigator();  // 체크인 화면부터 다시 시작
  } else if (step == 0) {
    nav.completeCheckIn();
  } else {
    nav.handle(kSteps[step - 1]);
  }

  const uint32_t t0 = millis();
  ui::renderScreen(display.canvas(), nav);
  const uint32_t t1 = millis();
  display.present();
  const uint32_t t2 = millis();

  Serial.printf("screen=%d draw=%lums present=%lums\n", static_cast<int>(nav.current()),
                static_cast<unsigned long>(t1 - t0), static_cast<unsigned long>(t2 - t1));

  step = (step >= kStepCount) ? -1 : step + 1;
  delay(2000);
}