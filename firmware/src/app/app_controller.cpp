#include "app/app_controller.h"

#include <string.h>

#include <string_view>
#include <vector>

#include "core/mission_catalog.h"
#include "core/mission_generator.h"

namespace app {
namespace {

constexpr const char* kSaveKey = "save";
constexpr uint32_t kTickIntervalMs = 1000;

static_assert(kSaveMissionSlots == ui::kMissionSlotCount, "저장 슬롯 수와 미션 슬롯 수가 같아야 합니다");

uint32_t dateKeyOf(const hal::ClockTime& t) {
  return static_cast<uint32_t>(t.year) * 10000u + static_cast<uint32_t>(t.month) * 100u + t.day;
}

void copyId(char* dst, const char* src) {
  strncpy(dst, src, kMissionIdSize - 1);
  dst[kMissionIdSize - 1] = '\0';
}

const maeumgochi::MissionDefinition* findMission(const char* id) {
  for (const auto& mission : maeumgochi::missionCatalog()) {
    if (strcmp(mission.id, id) == 0) return &mission;
  }
  return nullptr;
}

bool validCheckInValue(uint8_t value) {
  return value >= ui::kCheckInMin && value <= ui::kCheckInMax;
}

ui::MissionViewState toViewState(maeumgochi::MissionStatus status) {
  switch (status) {
    case maeumgochi::MissionStatus::NotStarted:
      return ui::MissionViewState::kNotStarted;
    case maeumgochi::MissionStatus::InProgress:
      return ui::MissionViewState::kInProgress;
    case maeumgochi::MissionStatus::Completed:
      return ui::MissionViewState::kCompleted;
  }
  return ui::MissionViewState::kNotStarted;
}

ui::MissionViewPeriod toViewPeriod(maeumgochi::MissionPeriod period) {
  switch (period) {
    case maeumgochi::MissionPeriod::Morning:
      return ui::MissionViewPeriod::kMorning;
    case maeumgochi::MissionPeriod::Afternoon:
      return ui::MissionViewPeriod::kAfternoon;
    case maeumgochi::MissionPeriod::Special:
      return ui::MissionViewPeriod::kSpecial;
  }
  return ui::MissionViewPeriod::kMorning;
}

}  // namespace

AppController::AppController(const Devices& devices, uint32_t randomSeed)
    : dev_(devices), rng_(randomSeed) {
  checkIn_.reset(false);
}

// ───────── 시작 / 주기 처리 ─────────

void AppController::begin(uint32_t nowMs) {
  nowMs_ = nowMs;
  if (dev_.storage) dev_.storage->begin();
  if (dev_.haptic) dev_.haptic->begin();
  if (dev_.light) dev_.light->begin();  // 실패해도 계속합니다. 드라이버가 다시 시도합니다.
  if (dev_.clock) dev_.clock->begin();

  refreshClock(/*detectNewDay=*/false);
  restore();

  lastTickMs_ = nowMs;
  lastSaveMs_ = nowMs;
  savedSunlightSeconds_ = sun_.seconds();
  redraw_ = true;
}

void AppController::update(uint32_t nowMs) {
  nowMs_ = nowMs;

  if (dev_.buttons) {
    dev_.buttons->update();
    bool pressed[hal::kButtonCount];
    for (uint8_t i = 0; i < hal::kButtonCount; ++i) {
      pressed[i] = dev_.buttons->isPressed(static_cast<hal::ButtonId>(i));
    }
    ui::NavInput input = ui::NavInput::kLeftShort;
    if (mapper_.update(pressed, nowMs, input)) {
      handleInput(input, nowMs);
    }
  }

  if (dev_.haptic) dev_.haptic->update();

  if (dev_.light) {
    dev_.light->update();
    sun_.update(dev_.light->latest(), nowMs);
  }

  if (nowMs - lastTickMs_ >= kTickIntervalMs) {
    lastTickMs_ = nowMs;
    tick();
  }
}

void AppController::tick() {
  refreshClock(/*detectNewDay=*/true);

  // 햇빛 시간은 자주 바뀌므로 플래시 마모를 줄이기 위해 간격을 두고 저장합니다.
  if (sun_.seconds() != savedSunlightSeconds_ && nowMs_ - lastSaveMs_ >= kSunlightSaveIntervalMs) {
    save();
  }
}

void AppController::refreshClock(bool detectNewDay) {
  hal::ClockTime t{};
  const bool ok = dev_.clock != nullptr && dev_.clock->now(t);

  if (ok != timeValid_ || (ok && (t.hour != hour_ || t.minute != minute_))) {
    redraw_ = true;
  }
  timeValid_ = ok;
  if (!ok) {
    return;
  }

  hour_ = t.hour;
  minute_ = t.minute;
  const uint32_t key = dateKeyOf(t);
  if (detectNewDay && dateKey_ != 0 && key != dateKey_) {
    dateKey_ = key;
    beginNewDay();
    save();
    redraw_ = true;
  } else {
    dateKey_ = key;
  }
}

void AppController::beginNewDay() {
  if (hasMissions_) {
    rememberTodayAsPrevious();
  }
  hasMissions_ = false;
  checkedInToday_ = false;
  mood_ = 0;
  energy_ = 0;
  missions_ = {};
  sun_.reset();
  savedSunlightSeconds_ = 0;
  nav_ = ui::ScreenNavigator();  // 체크인 화면부터 다시 시작합니다.
  checkIn_.reset(false);
}

void AppController::rememberTodayAsPrevious() {
  prevCount_ = 0;
  for (const auto& mission : missions_) {
    if (mission.definition != nullptr) {
      copyId(prevIds_[prevCount_++].data(), mission.definition->id);
    }
  }
}

// ───────── 입력 ─────────

void AppController::handleInput(ui::NavInput input, uint32_t nowMs) {
  nowMs_ = nowMs;
  redraw_ = true;

  if (nav_.current() == ui::Screen::kCheckIn) {
    handleCheckInInput(input);
    return;
  }

  const ui::Screen before = nav_.current();
  if (before == ui::Screen::kMission && input == ui::NavInput::kCenterShort) {
    advanceMission();
    return;
  }
  if (before == ui::Screen::kMain && input == ui::NavInput::kCenterShort) {
    nav_.setMissionIndex(currentMissionIndex());
  }

  nav_.handle(input);

  // 생활 메뉴의 "체크인"으로 다시 들어온 경우입니다. 오늘 이미 했다면 취소할 수 있습니다.
  if (nav_.current() == ui::Screen::kCheckIn && before != ui::Screen::kCheckIn) {
    checkIn_.reset(checkedInToday_);
  }
}

void AppController::handleCheckInInput(ui::NavInput input) {
  switch (checkIn_.handle(input)) {
    case ui::CheckInResult::kNone:
    case ui::CheckInResult::kUpdated:
      return;
    case ui::CheckInResult::kCancelled:
      nav_.completeCheckIn();
      return;
    case ui::CheckInResult::kCompleted:
      onCheckInCompleted();
      return;
  }
}

void AppController::onCheckInCompleted() {
  mood_ = checkIn_.mood();
  energy_ = checkIn_.energy();

  // 미션은 하루에 한 번만 배정합니다. 다시 체크인하면 기분과 에너지만 갱신합니다.
  if (!checkedInToday_ || !hasMissions_) {
    assignMissions();
  }
  checkedInToday_ = true;
  nav_.completeCheckIn();
  save();
}

void AppController::assignMissions() {
  std::vector<std::string_view> previous;
  for (uint8_t i = 0; i < prevCount_; ++i) {
    previous.emplace_back(prevIds_[i].data());
  }

  auto assignment = maeumgochi::assignDailyMissions(mood_, energy_, previous, rng_);
  if (!assignment) {
    // 전날 제외 때문에 후보가 비면 제외 없이 다시 뽑습니다.
    assignment = maeumgochi::assignDailyMissions(mood_, energy_, {}, rng_);
  }
  if (!assignment) {
    hasMissions_ = false;
    return;
  }

  for (size_t i = 0; i < missions_.size(); ++i) {
    missions_[i] = maeumgochi::createMissionProgress(*assignment->missions[i]);
  }
  hasMissions_ = true;
}

uint8_t AppController::currentMissionIndex() const {
  if (hasMissions_) {
    for (uint8_t i = 0; i < missions_.size(); ++i) {
      if (missions_[i].status != maeumgochi::MissionStatus::Completed) return i;
    }
  }
  return 0;
}

void AppController::advanceMission() {
  const uint8_t index = nav_.missionIndex();
  if (!hasMissions_ || index >= missions_.size()) {
    return;
  }

  maeumgochi::MissionProgress& mission = missions_[index];
  if (mission.status == maeumgochi::MissionStatus::NotStarted) {
    // TODO(역할 2 통합): 센서 미션이면 여기서 ActivityService::startMission()을 호출합니다.
    if (maeumgochi::startMission(mission)) {
      save();
    }
  } else if (mission.status == maeumgochi::MissionStatus::InProgress) {
    // 지금은 모든 미션을 가운데 버튼으로 직접 완료합니다. (센서가 없어도 쓸 수 있어야 하는 요구사항)
    // TODO(역할 2 통합): 센서 미션은 센서 결과가 조건을 넘었을 때만 완료하도록 바꿉니다.
    const maeumgochi::MissionCompletionResult result = maeumgochi::completeMission(mission, player_);
    if (result.completed) {
      buzz(result.characterGrew ? kHapticGrowthMs : kHapticCompleteMs);
      save();
    }
  }
}

void AppController::buzz(uint16_t durationMs) {
  if (dev_.haptic) dev_.haptic->vibrate(durationMs);
}

bool AppController::consumeRedraw() {
  const bool needed = redraw_;
  redraw_ = false;
  return needed;
}

// ───────── 화면용 데이터 ─────────

ui::ScreenViewModel AppController::viewModel() const {
  ui::ScreenViewModel vm;
  vm.timeValid = timeValid_;
  vm.hour = hour_;
  vm.minute = minute_;

  vm.coins = player_.coins;
  vm.characterStage = player_.characterStage;
  vm.missionsPerGrowth = maeumgochi::kMissionsPerCharacterGrowth;
  vm.nextGrowthIn =
      player_.characterStage >= maeumgochi::kCharacterStageCount
          ? 0
          : static_cast<uint8_t>(maeumgochi::kMissionsPerCharacterGrowth -
                                 player_.completedMissions % maeumgochi::kMissionsPerCharacterGrowth);

  vm.checkInStep = static_cast<uint8_t>(checkIn_.step());
  vm.checkInValue = checkIn_.value();

  vm.currentMission = currentMissionIndex();
  vm.selectedMission = nav_.missionIndex();
  if (hasMissions_) {
    vm.missionCount = ui::kMissionSlotCount;
    for (uint8_t i = 0; i < ui::kMissionSlotCount; ++i) {
      const maeumgochi::MissionProgress& mission = missions_[i];
      ui::MissionView& view = vm.missions[i];
      view.name = mission.definition->name;
      view.period = toViewPeriod(mission.definition->period);
      view.state = toViewState(mission.status);
      view.reward = maeumgochi::coinsForDifficulty(mission.definition->difficulty);
      view.sensorBased = mission.definition->completionMethod == maeumgochi::CompletionMethod::Sensor;
    }
  }
  return vm;
}

// ───────── 저장 / 복원 ─────────

void AppController::save() {
  lastSaveMs_ = nowMs_;
  savedSunlightSeconds_ = sun_.seconds();
  if (dev_.storage == nullptr) {
    return;
  }

  SaveData data;
  memset(&data, 0, sizeof(data));  // 패딩까지 0으로 채워 저장 내용을 항상 같게 합니다.
  data.version = kSaveVersion;
  data.characterStage = player_.characterStage;
  data.mood = checkedInToday_ ? mood_ : 0;
  data.energy = checkedInToday_ ? energy_ : 0;
  data.coins = player_.coins;
  data.completedMissions = player_.completedMissions;
  data.dateKey = dateKey_;
  data.sunlightSeconds = sun_.seconds();

  if (hasMissions_) {
    data.missionCount = kSaveMissionSlots;
    for (uint8_t i = 0; i < kSaveMissionSlots; ++i) {
      copyId(data.missionId[i], missions_[i].definition->id);
      data.missionStatus[i] = static_cast<uint8_t>(missions_[i].status);
    }
  }

  data.previousCount = prevCount_;
  for (uint8_t i = 0; i < prevCount_; ++i) {
    copyId(data.previousId[i], prevIds_[i].data());
  }

  dev_.storage->save(kSaveKey, &data, sizeof(data));
}

void AppController::restore() {
  SaveData data{};
  if (dev_.storage == nullptr || !dev_.storage->load(kSaveKey, &data, sizeof(data)) ||
      data.version != kSaveVersion) {
    return;  // 저장 데이터가 없거나 형식이 다르면 새로 시작합니다.
  }

  player_.coins = data.coins;
  player_.completedMissions = data.completedMissions;
  player_.characterStage = data.characterStage >= 1 && data.characterStage <= maeumgochi::kCharacterStageCount
                               ? data.characterStage
                               : maeumgochi::characterStageForCompletedMissions(data.completedMissions);

  prevCount_ = data.previousCount < kSaveMissionSlots ? data.previousCount : kSaveMissionSlots;
  for (uint8_t i = 0; i < prevCount_; ++i) {
    data.previousId[i][kMissionIdSize - 1] = '\0';
    copyId(prevIds_[i].data(), data.previousId[i]);
  }

  // 저장된 미션 id를 현재 카탈로그에서 찾습니다. 모르는 id가 있으면 오늘 미션은 버립니다.
  const maeumgochi::MissionDefinition* definitions[kSaveMissionSlots] = {};
  bool missionsOk = data.missionCount == kSaveMissionSlots;
  for (uint8_t i = 0; missionsOk && i < kSaveMissionSlots; ++i) {
    data.missionId[i][kMissionIdSize - 1] = '\0';
    definitions[i] = findMission(data.missionId[i]);
    missionsOk = definitions[i] != nullptr &&
                 data.missionStatus[i] <= static_cast<uint8_t>(maeumgochi::MissionStatus::Completed);
  }

  const bool newDay = dateKey_ != 0 && data.dateKey != 0 && data.dateKey != dateKey_;
  if (newDay) {
    // 날짜가 바뀌었습니다. 저장된 미션은 "전날 미션"이 되고 체크인부터 다시 합니다.
    if (missionsOk) {
      prevCount_ = kSaveMissionSlots;
      for (uint8_t i = 0; i < kSaveMissionSlots; ++i) {
        copyId(prevIds_[i].data(), data.missionId[i]);
      }
    }
    return;
  }

  sun_.restore(data.sunlightSeconds);

  if (missionsOk && validCheckInValue(data.mood) && validCheckInValue(data.energy)) {
    for (uint8_t i = 0; i < kSaveMissionSlots; ++i) {
      missions_[i] = maeumgochi::createMissionProgress(*definitions[i]);
      missions_[i].status = static_cast<maeumgochi::MissionStatus>(data.missionStatus[i]);
      missions_[i].rewardGranted = missions_[i].status == maeumgochi::MissionStatus::Completed;
    }
    hasMissions_ = true;
    checkedInToday_ = true;
    mood_ = data.mood;
    energy_ = data.energy;
    nav_.completeCheckIn();
  }
}

}  // namespace app
