#include <Arduino.h>

#include "drivers/gpio_buttons.h"
#include "ui/button_input.h"
#include "ui/screen_navigator.h"

namespace {
constexpr unsigned long kHeartbeatIntervalMs = 5000;
unsigned long lastHeartbeatMs = 0;

GpioButtons buttons;
ui::ButtonInput buttonInput;
ui::ScreenNavigator navigator;

void waitForSerial(unsigned long timeoutMs) {
  const unsigned long startedAt = millis();
  while (!Serial && millis() - startedAt < timeoutMs) {
    delay(10);
  }
}

void printBoardInfo() {
  Serial.println();
  Serial.println("=== Maeumgochi firmware ===");
  Serial.printf("Chip: %s, revision %d, %d core(s)\n", ESP.getChipModel(), ESP.getChipRevision(),
                ESP.getChipCores());
  Serial.printf("Flash: %lu MB\n", ESP.getFlashChipSize() / (1024U * 1024U));
  Serial.printf("PSRAM: %lu MB\n", ESP.getPsramSize() / (1024U * 1024U));
  Serial.println("Status: app skeleton is running");
}

void handleButtons() {
  buttons.update();

  ui::NavInput events[hal::kButtonCount];
  const uint8_t count = buttonInput.poll(buttons, millis(), events);
  for (uint8_t i = 0; i < count; ++i) {
    // 체크인 화면 로직은 5단계에서 구현합니다. 그동안은 가운데 버튼으로 통과시킵니다.
    if (navigator.current() == ui::Screen::kCheckIn && events[i] == ui::NavInput::kCenterShort) {
      navigator.completeCheckIn();
      Serial.printf("screen=%d menu=%d\n", static_cast<int>(navigator.current()),
                    navigator.menuIndex());
    } else if (navigator.handle(events[i])) {
      Serial.printf("screen=%d menu=%d\n", static_cast<int>(navigator.current()),
                    navigator.menuIndex());
    }
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  waitForSerial(2000);
  printBoardInfo();

  buttons.begin();
  Serial.printf("init: screen=%d menu=%d\n", static_cast<int>(navigator.current()),
                navigator.menuIndex());

  lastHeartbeatMs = millis();
}

void loop() {
  handleButtons();

  const unsigned long now = millis();
  if (now - lastHeartbeatMs >= kHeartbeatIntervalMs) {
    lastHeartbeatMs = now;
    Serial.printf("[heartbeat] uptime=%lu s, free_heap=%lu bytes\n", now / 1000, ESP.getFreeHeap());
  }

  delay(10);
}