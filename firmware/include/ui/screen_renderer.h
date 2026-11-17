#pragma once

class Adafruit_GFX;

namespace ui {

class ScreenNavigator;
struct ScreenViewModel;

// 현재 내비게이션 상태에 해당하는 240x240 화면을 그립니다. 시간, 코인, 미션 등은 viewModel에서 가져옵니다.
void renderScreen(Adafruit_GFX& display, const ScreenNavigator& navigator,
                  const ScreenViewModel& viewModel);

// 예시값(전달받은 UI 아트의 데모 데이터)으로 그립니다. PC 미리보기와 st7789_check용입니다.
void renderScreen(Adafruit_GFX& display, const ScreenNavigator& navigator);

}  // namespace ui
