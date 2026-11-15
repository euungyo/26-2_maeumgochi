#include "ui/screen_renderer.h"

#include "ui/art/screens_demo.h"
#include "ui/screen_navigator.h"

namespace ui {

void renderScreen(Adafruit_GFX& display, const ScreenNavigator& navigator) {
  switch (navigator.current()) {
    case Screen::kCheckIn:
      maeum_screen::checkinScreen(display);
      break;
    case Screen::kMain:
      maeum_screen::mainScreen(display);
      break;
    case Screen::kLifeMenu:
      maeum_screen::lifeMenuScreen(display, navigator.menuIndex());
      break;
    case Screen::kCharacterMenu:
      maeum_screen::charMenuScreen(display, navigator.menuIndex());
      break;
    case Screen::kMission:
      maeum_screen::missionDetailScreen(display);
      break;
    case Screen::kMissionList:
      maeum_screen::missionListScreen(display);
      break;
    case Screen::kRecord:
      maeum_screen::recordScreen(display);
      break;
    case Screen::kStatus:
      maeum_screen::growthScreen(display);
      break;
    case Screen::kShop:
      maeum_screen::shopListScreen(display);
      break;
    case Screen::kInventory:
      maeum_screen::bagScreen(display);
      break;
  }
}

}  // namespace ui
