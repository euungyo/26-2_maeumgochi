#include <Arduino.h>
#include <Wire.h>

namespace {
constexpr unsigned long kScanIntervalMs = 3000;

void waitForSerial(unsigned long timeoutMs) {
  const unsigned long startedAt = millis();
  while (!Serial && millis() - startedAt < timeoutMs) {
    delay(10);
  }
}

void scanI2cBus() {
  int found = 0;
  Serial.println("Scanning I2C bus...");

  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    const uint8_t error = Wire.endTransmission();

    if (error == 0) {
      Serial.printf("  found device at 0x%02X\n", address);
      ++found;
    } else if (error == 4) {
      Serial.printf("  unknown error at 0x%02X\n", address);
    }
  }

  if (found == 0) {
    Serial.println("  no I2C devices found");
  } else {
    Serial.printf("  %d device(s) found\n", found);
  }
  Serial.println();
}
}  // namespace

void setup() {
  Serial.begin(115200);
  waitForSerial(2000);

  Wire.begin(SDA, SCL);
  Wire.setClock(100000);

  Serial.println();
  Serial.println("=== Maeumgochi I2C scanner ===");
  Serial.printf("SDA=GPIO%d, SCL=GPIO%d\n", SDA, SCL);
  Serial.println("Power off the board before changing any wiring.");
}

void loop() {
  scanI2cBus();
  delay(kScanIntervalMs);
}
