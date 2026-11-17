#include <Arduino.h>
#include <Wire.h>

#include "app/app_controller.h"
#include "drivers/board_pins.h"
#include "drivers/gpio_buttons.h"
#include "drivers/gpio_haptic.h"
#include "drivers/ltr390_sensor.h"
#include "drivers/nvs_storage.h"
#include "drivers/pcf8523_clock.h"
#include "drivers/st7789_display.h"
#include "ui/screen_renderer.h"

namespace {

drivers::St7789Config makeDisplayConfig() {
  drivers::St7789Config config;
  config.sck = board::kLcdSck;
  config.mosi = board::kLcdMosi;
  config.rst = board::kLcdRes;
  config.dc = board::kLcdDc;
  config.cs = board::kLcdCs;
  return config;
}

drivers::St7789Display display(makeDisplayConfig());
drivers::GpioButtons buttons(board::kButtonLeft, board::kButtonCenter, board::kButtonRight);
drivers::GpioHaptic haptic(board::kHapticGate);
drivers::Ltr390Sensor light;
drivers::Pcf8523Clock rtc;
drivers::NvsStorage storage;

app::AppController* controller = nullptr;
bool displayReady = false;

void waitForSerial(unsigned long timeoutMs) {
  const unsigned long startedAt = millis();
  while (!Serial && millis() - startedAt < timeoutMs) {
    delay(10);
  }
}

}  // namespace

void setup() {
  Serial.begin(115200);
  waitForSerial(2000);
  Serial.println();
  Serial.println("=== Maeumgochi firmware ===");

  Wire.begin(SDA, SCL);
  buttons.begin();

  displayReady = display.begin();
  if (!displayReady) {
    Serial.println("ERROR: display.begin() failed. Running without a screen.");
  }

  app::Devices devices;
  devices.storage = &storage;
  devices.haptic = &haptic;
  devices.light = &light;  // 센서가 없어도 컨트롤러가 계속 동작합니다.
  devices.buttons = &buttons;
  devices.clock = &rtc;
  controller = new app::AppController(devices, esp_random());
  controller->begin(millis());
}

void loop() {
  controller->update(millis());

  if (displayReady && controller->consumeRedraw()) {
    ui::renderScreen(display.canvas(), controller->navigator(), controller->viewModel());
    display.present();
  }
  delay(5);
}
