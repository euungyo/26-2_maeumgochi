#include "ui/screen_renderer.h"

#include "ui/art/screens_demo.h"
#include "ui/screen_navigator.h"
#include "ui/view_model.h"

namespace ui {
namespace {

ScreenViewModel demoViewModel() {
  ScreenViewModel vm;
  vm.timeValid = true;
  vm.hour = 9;
  vm.minute = 24;
  vm.coins = 120;
  vm.characterStage = 2;
  vm.nextGrowthIn = 1;
  vm.checkInValue = 4;
  vm.missionCount = kMissionSlotCount;
  vm.currentMission = 1;
  vm.selectedMission = 1;
  vm.missions[0].name = "침대 정리";
  vm.missions[0].state = MissionViewState::kCompleted;
  vm.missions[1].name = "스트레칭";
  vm.missions[1].sensorBased = true;
  vm.missions[1].reward = 200;
  vm.missions[2].name = "산책하기";
  vm.missions[2].period = MissionViewPeriod::kAfternoon;
  return vm;
}

bool allMissionsDone(const ScreenViewModel& vm) {
  for (uint8_t i = 0; i < vm.missionCount; ++i) {
    if (vm.missions[i].state != MissionViewState::kCompleted) return false;
  }
  return vm.missionCount > 0;
}

void drawMain(Adafruit_GFX& display, const ScreenViewModel& vm) {
  const char* name = "";
  if (vm.missionCount > 0) {
    name = allMissionsDone(vm) ? maeum_screen::allDoneText() : vm.missions[vm.currentMission].name;
  }
  maeum_screen::mainScreen(display, vm.hour, vm.minute, vm.timeValid, vm.coins, name,
                           vm.characterStage);
}

void drawMissionList(Adafruit_GFX& display, const ScreenNavigator& navigator,
                     const ScreenViewModel& vm) {
  maeum_screen::MissionRow rows[kMissionSlotCount] = {};
  for (uint8_t i = 0; i < vm.missionCount; ++i) {
    rows[i].name = vm.missions[i].name;
    rows[i].tag = maeum_screen::periodTag(static_cast<int>(vm.missions[i].period));
    rows[i].done = vm.missions[i].state == MissionViewState::kCompleted;
  }
  maeum_screen::missionListScreen(display, rows, vm.missionCount, navigator.menuIndex());
}

void drawMissionDetail(Adafruit_GFX& display, const ScreenNavigator& navigator,
                       const ScreenViewModel& vm) {
  maeum_screen::MissionDetail detail;
  const uint8_t index = navigator.missionIndex();
  if (index < vm.missionCount) {
    const MissionView& mission = vm.missions[index];
    detail.name = mission.name;
    detail.state = static_cast<int>(mission.state);
    detail.reward = mission.reward;
    detail.sensor = mission.sensorBased;
  } else {
    detail.name = "";
    detail.state = 0;
    detail.reward = 0;
  }
  detail.index = index;
  detail.count = vm.missionCount;
  detail.stage = vm.characterStage;
  maeum_screen::missionDetailScreen(display, detail);
}

}  // namespace

void renderScreen(Adafruit_GFX& display, const ScreenNavigator& navigator,
                  const ScreenViewModel& vm) {
  switch (navigator.current()) {
    case Screen::kCheckIn:
      maeum_screen::checkinScreen(display, vm.checkInStep, vm.checkInValue);
      break;
    case Screen::kMain:
      drawMain(display, vm);
      break;
    case Screen::kLifeMenu:
      maeum_screen::lifeMenuScreen(display, navigator.menuIndex());
      break;
    case Screen::kCharacterMenu:
      maeum_screen::charMenuScreen(display, navigator.menuIndex());
      break;
    case Screen::kMission:
      drawMissionDetail(display, navigator, vm);
      break;
    case Screen::kMissionList:
      drawMissionList(display, navigator, vm);
      break;
    case Screen::kRecord:
      maeum_screen::recordScreen(display);  // 기록 데이터는 아직 예시값입니다.
      break;
    case Screen::kStatus:
      maeum_screen::growthScreen(display, vm.characterStage, vm.nextGrowthIn, vm.missionsPerGrowth);
      break;
    case Screen::kShop:
      maeum_screen::shopListScreen(display);  // 상점 데이터는 아직 예시값입니다.
      break;
    case Screen::kInventory:
      maeum_screen::bagScreen(display);  // 인벤토리 데이터는 아직 예시값입니다.
      break;
  }
}

void renderScreen(Adafruit_GFX& display, const ScreenNavigator& navigator) {
  renderScreen(display, navigator, demoViewModel());
}

}  // namespace ui
