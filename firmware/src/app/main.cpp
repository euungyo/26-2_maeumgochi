#include <Arduino.h>

namespace {
constexpr unsigned long kHeartbeatIntervalMs = 5000;
unsigned long lastHeartbeatMs = 0;

void waitForSerial(unsigned long timeoutMs) {
  const unsigned long startedAt = millis();
  while (!Serial && millis() - startedAt < timeoutMs) {
    delay(10);
  }
}

void printBoardInfo() {
  Serial.println();
  Serial.println("=== Maeumgochi firmware ===");
  Serial.printf("Chip: %s, revision %d, %d core(s)\n", ESP.getChipModel(),
                ESP.getChipRevision(), ESP.getChipCores());
  Serial.printf("Flash: %lu MB\n", ESP.getFlashChipSize() / (1024U * 1024U));
  Serial.printf("PSRAM: %lu MB\n", ESP.getPsramSize() / (1024U * 1024U));
  Serial.println("Status: app skeleton is running");
}
}  // namespace

void setup() {
  Serial.begin(115200);
  waitForSerial(2000);
  printBoardInfo();
  lastHeartbeatMs = millis();
}

void loop() {
  const unsigned long now = millis();
  if (now - lastHeartbeatMs >= kHeartbeatIntervalMs) {
    lastHeartbeatMs = now;
    Serial.printf("[heartbeat] uptime=%lu s, free_heap=%lu bytes\n", now / 1000,
                  ESP.getFreeHeap());
  }

  delay(10);
}
