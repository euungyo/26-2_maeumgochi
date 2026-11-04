#include <Arduino.h>
#include <TinyGPSPlus.h>

namespace {
constexpr uint32_t kGnssBaud = 9600;
constexpr unsigned long kStatusIntervalMs = 5000;
TinyGPSPlus gps;
unsigned long lastStatusMs = 0;

void waitForSerial(unsigned long timeoutMs) {
  const unsigned long startedAt = millis();
  while (!Serial && millis() - startedAt < timeoutMs) {
    delay(10);
  }
}

void printStatus() {
  Serial.printf("chars=%lu satellites=", gps.charsProcessed());
  if (gps.satellites.isValid()) {
    Serial.print(gps.satellites.value());
  } else {
    Serial.print("unknown");
  }

  Serial.print(" location=");
  if (gps.location.isValid()) {
    Serial.printf("%.6f,%.6f", gps.location.lat(), gps.location.lng());
  } else {
    Serial.print("no-fix");
  }
  Serial.println();
}
}  // namespace

void setup() {
  Serial.begin(115200);
  waitForSerial(2000);

  // L76K TX -> XIAO RX(D7/GPIO44), L76K RX -> XIAO TX(D6/GPIO43)
  Serial1.begin(kGnssBaud, SERIAL_8N1, RX, TX);

  Serial.println();
  Serial.println("=== L76K GNSS diagnostic ===");
  Serial.printf("UART baud=%lu, RX=GPIO%d, TX=GPIO%d\n", kGnssBaud, RX, TX);
  Serial.println("Place the antenna outdoors or near a clear window and wait for a fix.");
}

void loop() {
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  const unsigned long now = millis();
  if (now - lastStatusMs >= kStatusIntervalMs) {
    lastStatusMs = now;
    printStatus();

    if (now > 10000 && gps.charsProcessed() < 10) {
      Serial.println("WARNING: no NMEA data; check module direction, UART pins, and antenna.");
    }
  }
}
