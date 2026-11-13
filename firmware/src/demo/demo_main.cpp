#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <Arduino.h>
#include <SPI.h>

#include "ui/button_gesture.h"
#include "ui/screen_navigator.h"
#include "ui/screen_renderer.h"

namespace {

// ---- LCD 핀 (디스플레이 담당 팀원 확정값과 반드시 일치시킬 것) ----
constexpr int8_t kTftCs = D2;
constexpr int8_t kTftDc = D3;
constexpr int8_t kTftRst = D1;
constexpr int8_t kSpiSck = D8;
constexpr int8_t kSpiMosi = D10;

// ---- 버튼 핀: 왼쪽, 가운데, 오른쪽 (GND와 사이에 연결, INPUT_PULLUP) ----
constexpr uint8_t kButtonPins[3] = {D0, D4, D6};
constexpr int kLeft = 0, kCenter = 1, kRight = 2;

// 가상 GND 핀 정의 (상수 배열만 전역/네임스페이스 내부에 선언 가능)
constexpr uint8_t kGroundPins[2] = {D5, D9};

Adafruit_ST7789 tft(&SPI, kTftCs, kTftDc, kTftRst);
GFXcanvas16* canvas = nullptr;  // 메모리에 먼저 그리고 한 번에 전송 (깜빡임 방지)

ui::ScreenNavigator nav;
ui::ButtonGesture gestures[3];

void redraw() {
  if (canvas != nullptr && canvas->getBuffer() != nullptr) {
    ui::renderScreen(*canvas, nav);
    tft.drawRGBBitmap(0, 0, canvas->getBuffer(), 240, 240);
  } else {
    ui::renderScreen(tft, nav);  // 캔버스 메모리 할당 실패 시 직접 그리기
  }
}

const char* screenName(ui::Screen screen) {
  switch (screen) {
    case ui::Screen::kCheckIn:       return "체크인";
    case ui::Screen::kMain:          return "메인";
    case ui::Screen::kLifeMenu:      return "생활 메뉴";
    case ui::Screen::kCharacterMenu: return "캐릭터 메뉴";
    case ui::Screen::kMission:       return "미션 상세";
    case ui::Screen::kMissionList:   return "미션 목록";
    case ui::Screen::kRecord:        return "기록";
    case ui::Screen::kStatus:        return "성장";
    case ui::Screen::kShop:          return "상점";
    case ui::Screen::kInventory:     return "가방";
  }
  return "?";
}

// 메뉴 화면에서 현재 강조된 항목 이름입니다. 메뉴가 아니면 nullptr.
const char* selectedItemName(ui::Screen screen, uint8_t index) {
  static const char* const kLife[] = {"미션", "체크인", "기록"};
  static const char* const kCharacter[] = {"성장", "상점", "가방"};
  if (screen == ui::Screen::kLifeMenu && index < 3) return kLife[index];
  if (screen == ui::Screen::kCharacterMenu && index < 3) return kCharacter[index];
  return nullptr;
}

const char* buttonName(int button, ui::GestureEvent ev) {
  const bool isLong = (ev == ui::GestureEvent::kLongPress);
  if (button == kLeft) return "왼쪽 짧게";
  if (button == kRight) return "오른쪽 짧게";
  return isLong ? "가운데 길게" : "가운데 짧게";
}

void logTransition(int button, ui::GestureEvent ev, ui::Screen before,
                   ui::Screen after) {
  const char* item = selectedItemName(after, nav.menuIndex());
  Serial.printf("[%s] ", buttonName(button, ev));
  if (before != after) {
    Serial.printf("%s → %s", screenName(before), screenName(after));
  } else {
    Serial.printf("%s", screenName(after));
  }
  if (item != nullptr) Serial.printf("  (선택: %s)", item);
  Serial.println();
}

// 버튼 이벤트를 내비게이터 입력으로 바꿉니다. 처리할 입력이면 true.
bool dispatch(int button, ui::GestureEvent ev) {
  if (ev == ui::GestureEvent::kShortPress) {
    if (button == kLeft) nav.handle(ui::NavInput::kLeftShort);
    else if (button == kCenter) nav.handle(ui::NavInput::kCenterShort);
    else nav.handle(ui::NavInput::kRightShort);
    return true;
  }
  if (ev == ui::GestureEvent::kLongPress && button == kCenter) {
    nav.handle(ui::NavInput::kCenterLong);
    return true;
  }
  return false;  // 왼쪽/오른쪽 길게 누름은 사용하지 않음
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);

  // MISO(D9)를 SPI에 연결하지 않도록 -1로 지정한 뒤 LCD 초기화
  SPI.begin(kSpiSck, -1, kSpiMosi, -1);
  tft.init(240, 240);
  tft.setRotation(0);  // 방향이 틀리면 0~3으로 바꿔 확인

  // 버튼은 SPI 초기화 이후에 설정
  for (uint8_t pin : kButtonPins) pinMode(pin, INPUT_PULLUP);

  // [수정 완료] 가상 GND 핀 설정 코드를 올바르게 setup 함수 안으로 이동시켰습니다.
  for (uint8_t pin : kGroundPins) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  canvas = new GFXcanvas16(240, 240);  // 약 115KB
  Serial.printf("canvas=%s, free_heap=%lu\n",
                (canvas && canvas->getBuffer()) ? "ok" : "FAILED",
                static_cast<unsigned long>(ESP.getFreeHeap()));

  nav.completeCheckIn();  // 체크인 입력은 미구현이므로 메인에서 시작
  redraw();
  Serial.printf("=== 마음고치 데모 시작: %s 화면 ===\n", screenName(nav.current()));
}

void loop() {
  const uint32_t now = millis();
  bool needRedraw = false;

  for (int i = 0; i < 3; ++i) {
    const bool pressed = digitalRead(kButtonPins[i]) == LOW;
    const ui::GestureEvent ev = gestures[i].update(pressed, now);
    if (ev == ui::GestureEvent::kNone) continue;

    const ui::Screen before = nav.current();
    if (dispatch(i, ev)) {
      logTransition(i, ev, before, nav.current());
      needRedraw = true;
    }
  }

  if (needRedraw) redraw();
  delay(1);
}