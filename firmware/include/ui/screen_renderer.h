#pragma once

class Adafruit_GFX;

namespace ui {

class ScreenNavigator;

// 현재 내비게이션 상태에 해당하는 240x240 화면을 그립니다.
void renderScreen(Adafruit_GFX& display, const ScreenNavigator& navigator);

}  // namespace ui
