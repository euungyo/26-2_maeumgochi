#pragma once

#include <stdint.h>

#include <array>
#include <random>

#include "activity/sunlight_tracker.h"
#include "app/save_data.h"
#include "core/mission_progression.h"
#include "hal/buttons.h"
#include "hal/clock.h"
#include "hal/haptic.h"
#include "hal/light_sensor.h"
#include "hal/storage.h"
#include "ui/checkin_flow.h"
#include "ui/nav_input_mapper.h"
#include "ui/screen_navigator.h"
#include "ui/view_model.h"

namespace app {

// 장치는 모두 소유하지 않고 가리키기만 합니다. 없는 장치는 nullptr로 두면 그 기능만 빠집니다.
struct Devices {
  hal::Storage* storage = nullptr;
  hal::Haptic* haptic = nullptr;
  hal::LightSensor* light = nullptr;
  hal::Buttons* buttons = nullptr;
  hal::Clock* clock = nullptr;
};

constexpr uint32_t kSunlightSaveIntervalMs = 5UL * 60UL * 1000UL;  // 햇빛 시간 저장 최소 간격
constexpr uint16_t kHapticCompleteMs = 200;
constexpr uint16_t kHapticGrowthMs = 600;

// 앱 전체 흐름: 부팅 복원 -> 체크인 -> 미션 배정 -> 진행/완료 -> 보상/성장 -> 저장.
// Arduino에 의존하지 않아 PC 단위 테스트에서 가짜 장치로 실행할 수 있습니다.
class AppController {
 public:
  AppController(const Devices& devices, uint32_t randomSeed);

  // 장치를 초기화하고 저장 데이터를 복원합니다. 센서나 시계가 없어도 계속 진행합니다.
  void begin(uint32_t nowMs);
  // 메인 루프에서 자주 호출합니다. (버튼, 진동, 센서, 날짜 변경, 주기 저장)
  void update(uint32_t nowMs);
  // 버튼 대신 입력을 직접 넣습니다. update()도 내부에서 이 함수를 씁니다.
  void handleInput(ui::NavInput input, uint32_t nowMs);

  const ui::ScreenNavigator& navigator() const { return nav_; }
  ui::ScreenViewModel viewModel() const;
  // 화면을 다시 그려야 하면 true를 반환하고 표시를 지웁니다.
  bool consumeRedraw();

  uint32_t sunlightSeconds() const { return sun_.seconds(); }
  const maeumgochi::PlayerProgress& player() const { return player_; }

 private:
  void handleCheckInInput(ui::NavInput input);
  void onCheckInCompleted();
  void assignMissions();
  void advanceMission();
  uint8_t currentMissionIndex() const;

  void refreshClock(bool detectNewDay);
  void tick();
  void beginNewDay();
  void rememberTodayAsPrevious();

  void restore();
  void save();
  void buzz(uint16_t durationMs);

  Devices dev_;
  std::mt19937 rng_;
  ui::ScreenNavigator nav_;
  ui::NavInputMapper mapper_;
  ui::CheckInFlow checkIn_;
  activity::SunlightTracker sun_;

  maeumgochi::PlayerProgress player_;
  std::array<maeumgochi::MissionProgress, ui::kMissionSlotCount> missions_{};
  bool hasMissions_ = false;
  bool checkedInToday_ = false;
  uint8_t mood_ = 0;
  uint8_t energy_ = 0;

  std::array<std::array<char, kMissionIdSize>, ui::kMissionSlotCount> prevIds_{};
  uint8_t prevCount_ = 0;

  bool timeValid_ = false;
  uint8_t hour_ = 0;
  uint8_t minute_ = 0;
  uint32_t dateKey_ = 0;

  uint32_t nowMs_ = 0;
  uint32_t lastTickMs_ = 0;
  uint32_t lastSaveMs_ = 0;
  uint32_t savedSunlightSeconds_ = 0;
  bool redraw_ = true;
};

}  // namespace app
