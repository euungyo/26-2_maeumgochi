#include <unity.h>

#include <string.h>

#include <map>
#include <string>
#include <vector>

#include "app/app_controller.h"
#include "core/mission_catalog.h"

namespace {

using ui::NavInput;
using ui::Screen;

class FakeStorage : public hal::Storage {
 public:
  bool begin() override { return true; }
  bool save(const char* key, const void* data, size_t size) override {
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    items_[key] = std::vector<unsigned char>(bytes, bytes + size);
    return true;
  }
  bool load(const char* key, void* data, size_t size) override {
    auto it = items_.find(key);
    if (it == items_.end() || it->second.size() != size) return false;
    memcpy(data, it->second.data(), size);
    return true;
  }
  bool exists(const char* key) override { return items_.count(key) > 0; }
  bool remove(const char* key) override { return items_.erase(key) > 0; }

 private:
  std::map<std::string, std::vector<unsigned char>> items_;
};

class FakeHaptic : public hal::Haptic {
 public:
  bool begin() override { return true; }
  void vibrate(uint16_t durationMs) override {
    ++count;
    last = durationMs;
  }
  void stop() override {}
  void update() override {}
  int count = 0;
  uint16_t last = 0;
};

class FakeLight : public hal::LightSensor {
 public:
  bool begin() override { return valid; }
  void update() override {}
  bool isValid() const override { return valid; }
  hal::LightReading latest() const override { return {lux, 0.0f, valid}; }
  float lux = 0.0f;
  bool valid = true;
};

class FakeClock : public hal::Clock {
 public:
  bool begin() override { return ok; }
  bool now(hal::ClockTime& out) override {
    if (!ok) return false;
    out = t;
    return true;
  }
  hal::ClockTime t = {2026, 11, 17, 9, 30, 0};
  bool ok = true;
};

struct Rig {
  FakeStorage storage;
  FakeHaptic haptic;
  FakeLight light;
  FakeClock clock;

  app::Devices devices() {
    app::Devices d;
    d.storage = &storage;
    d.haptic = &haptic;
    d.light = &light;
    d.clock = &clock;
    return d;
  }
};

void pick(app::AppController& c, int target) {
  while (c.viewModel().checkInValue < target) c.handleInput(NavInput::kRightShort, 0);
  while (c.viewModel().checkInValue > target) c.handleInput(NavInput::kLeftShort, 0);
}

void doCheckIn(app::AppController& c, int mood, int energy) {
  pick(c, mood);
  c.handleInput(NavInput::kCenterShort, 0);
  pick(c, energy);
  c.handleInput(NavInput::kCenterShort, 0);
}

// 현재 화면이 미션 상세일 때 가운데 버튼을 n번 누릅니다.
void press(app::AppController& c, NavInput input, int times = 1) {
  for (int i = 0; i < times; ++i) c.handleInput(input, 0);
}

void test_boot_shows_checkin_screen() {
  Rig rig;
  app::AppController c(rig.devices(), 1);
  c.begin(0);
  TEST_ASSERT_EQUAL(Screen::kCheckIn, c.navigator().current());
  TEST_ASSERT_EQUAL_UINT8(0, c.viewModel().missionCount);
  TEST_ASSERT_TRUE(c.viewModel().timeValid);
  TEST_ASSERT_EQUAL_UINT8(9, c.viewModel().hour);
  TEST_ASSERT_EQUAL_UINT8(30, c.viewModel().minute);
}

void test_checkin_assigns_three_missions_and_opens_main() {
  Rig rig;
  app::AppController c(rig.devices(), 1);
  c.begin(0);
  doCheckIn(c, 1, 5);

  TEST_ASSERT_EQUAL(Screen::kMain, c.navigator().current());
  const ui::ScreenViewModel vm = c.viewModel();
  TEST_ASSERT_EQUAL_UINT8(3, vm.missionCount);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ui::MissionViewPeriod::kMorning), static_cast<int>(vm.missions[0].period));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ui::MissionViewPeriod::kAfternoon), static_cast<int>(vm.missions[1].period));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ui::MissionViewPeriod::kSpecial), static_cast<int>(vm.missions[2].period));
  for (int i = 0; i < 3; ++i) TEST_ASSERT_TRUE(vm.missions[i].reward >= 100);
}

void test_mission_start_complete_pays_coins_and_vibrates() {
  Rig rig;
  app::AppController c(rig.devices(), 1);
  c.begin(0);
  doCheckIn(c, 3, 3);
  const uint16_t reward0 = c.viewModel().missions[0].reward;

  press(c, NavInput::kCenterShort);  // 메인 -> 현재 미션(0번)
  TEST_ASSERT_EQUAL(Screen::kMission, c.navigator().current());
  TEST_ASSERT_EQUAL_UINT8(0, c.navigator().missionIndex());

  press(c, NavInput::kCenterShort);  // 시작
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ui::MissionViewState::kInProgress),
                        static_cast<int>(c.viewModel().missions[0].state));
  TEST_ASSERT_EQUAL_UINT32(0, c.viewModel().coins);

  press(c, NavInput::kCenterShort);  // 완료
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ui::MissionViewState::kCompleted),
                        static_cast<int>(c.viewModel().missions[0].state));
  TEST_ASSERT_EQUAL_UINT32(reward0, c.viewModel().coins);
  TEST_ASSERT_EQUAL_INT(1, rig.haptic.count);
  TEST_ASSERT_EQUAL_UINT16(app::kHapticCompleteMs, rig.haptic.last);

  press(c, NavInput::kCenterShort);  // 이미 완료한 미션은 다시 보상하지 않음
  TEST_ASSERT_EQUAL_UINT32(reward0, c.viewModel().coins);
  TEST_ASSERT_EQUAL_INT(1, rig.haptic.count);
}

void test_second_completion_grows_character_with_longer_vibration() {
  Rig rig;
  app::AppController c(rig.devices(), 1);
  c.begin(0);
  doCheckIn(c, 3, 3);

  press(c, NavInput::kCenterShort, 3);  // 미션 0 완료
  press(c, NavInput::kCenterLong);       // 메인
  TEST_ASSERT_EQUAL(Screen::kMain, c.navigator().current());
  TEST_ASSERT_EQUAL_UINT8(1, c.viewModel().characterStage);
  TEST_ASSERT_EQUAL_UINT8(1, c.viewModel().nextGrowthIn);
  TEST_ASSERT_EQUAL_UINT8(1, c.viewModel().currentMission);

  press(c, NavInput::kCenterShort, 3);  // 현재 미션(1번) 시작과 완료
  TEST_ASSERT_EQUAL_UINT8(1, c.navigator().missionIndex());
  TEST_ASSERT_EQUAL_UINT8(2, c.viewModel().characterStage);
  TEST_ASSERT_EQUAL_UINT16(app::kHapticGrowthMs, rig.haptic.last);
}

void test_progress_is_restored_after_reboot() {
  Rig rig;
  uint32_t coins = 0;
  {
    app::AppController c(rig.devices(), 1);
    c.begin(0);
    doCheckIn(c, 4, 2);
    press(c, NavInput::kCenterShort, 3);   // 0번 완료
    press(c, NavInput::kCenterLong);
    press(c, NavInput::kCenterShort, 2);   // 1번 시작만
    coins = c.viewModel().coins;
    TEST_ASSERT_TRUE(coins > 0);
  }

  app::AppController again(rig.devices(), 99);
  again.begin(0);
  TEST_ASSERT_EQUAL(Screen::kMain, again.navigator().current());
  const ui::ScreenViewModel vm = again.viewModel();
  TEST_ASSERT_EQUAL_UINT32(coins, vm.coins);
  TEST_ASSERT_EQUAL_UINT8(3, vm.missionCount);
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ui::MissionViewState::kCompleted), static_cast<int>(vm.missions[0].state));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ui::MissionViewState::kInProgress), static_cast<int>(vm.missions[1].state));
  TEST_ASSERT_EQUAL_INT(static_cast<int>(ui::MissionViewState::kNotStarted), static_cast<int>(vm.missions[2].state));
  TEST_ASSERT_EQUAL_UINT8(1, vm.currentMission);
}

void test_new_day_requires_checkin_and_does_not_repeat_yesterdays_missions() {
  for (uint32_t seed = 0; seed < 20; ++seed) {
    Rig rig;  // 매번 "어제 저장된 상태"에서 시작합니다.
    std::string yesterday[3];
    {
      app::AppController c(rig.devices(), seed + 100);
      c.begin(0);
      doCheckIn(c, 3, 3);
      for (int i = 0; i < 3; ++i) yesterday[i] = c.viewModel().missions[i].name;
    }

    rig.clock.t.day = 18;
    app::AppController next(rig.devices(), seed);
    next.begin(0);
    TEST_ASSERT_EQUAL(Screen::kCheckIn, next.navigator().current());
    doCheckIn(next, 3, 3);
    const ui::ScreenViewModel vm = next.viewModel();
    for (int i = 0; i < 3; ++i) TEST_ASSERT_TRUE(yesterday[i] != vm.missions[i].name);
  }
}

void test_midnight_rollover_while_running() {
  Rig rig;
  app::AppController c(rig.devices(), 1);
  c.begin(0);
  doCheckIn(c, 3, 3);
  TEST_ASSERT_EQUAL(Screen::kMain, c.navigator().current());

  rig.clock.t.day = 18;
  rig.clock.t.hour = 0;
  c.update(1000);
  TEST_ASSERT_EQUAL(Screen::kCheckIn, c.navigator().current());
  TEST_ASSERT_EQUAL_UINT8(0, c.viewModel().missionCount);
  TEST_ASSERT_EQUAL_UINT32(0, c.sunlightSeconds());
}

void test_sunlight_accumulates_and_is_restored_same_day() {
  Rig rig;
  rig.light.lux = 20000.0f;
  {
    app::AppController c(rig.devices(), 1);
    c.begin(0);
    for (uint32_t t = 1000; t <= 310000; t += 1000) c.update(t);
    TEST_ASSERT_TRUE(c.sunlightSeconds() >= 300);
  }
  app::AppController again(rig.devices(), 1);
  again.begin(0);
  TEST_ASSERT_TRUE(again.sunlightSeconds() >= 290);  // 마지막 저장 시점까지

  rig.clock.t.day = 18;  // 다음 날에는 0부터
  app::AppController nextDay(rig.devices(), 1);
  nextDay.begin(0);
  TEST_ASSERT_EQUAL_UINT32(0, nextDay.sunlightSeconds());
}

void test_runs_without_optional_devices() {
  Rig rig;
  app::Devices d;
  d.storage = &rig.storage;
  app::AppController c(d, 1);
  c.begin(0);
  TEST_ASSERT_FALSE(c.viewModel().timeValid);
  doCheckIn(c, 5, 5);
  press(c, NavInput::kCenterShort, 3);
  TEST_ASSERT_TRUE(c.viewModel().coins > 0);
  c.update(5000);

  app::AppController bare(app::Devices(), 1);  // 저장소조차 없어도 멈추지 않음
  bare.begin(0);
  doCheckIn(bare, 3, 3);
  press(bare, NavInput::kCenterShort, 3);
  bare.update(5000);
  TEST_ASSERT_TRUE(bare.viewModel().coins > 0);
}

void test_missing_light_sensor_does_not_block_other_screens() {
  Rig rig;
  rig.light.valid = false;
  app::AppController c(rig.devices(), 1);
  c.begin(0);
  doCheckIn(c, 3, 3);
  for (uint32_t t = 1000; t <= 5000; t += 1000) c.update(t);
  TEST_ASSERT_EQUAL_UINT32(0, c.sunlightSeconds());
  press(c, NavInput::kRightShort);  // 캐릭터 메뉴
  TEST_ASSERT_EQUAL(Screen::kCharacterMenu, c.navigator().current());
}

void test_recheckin_keeps_todays_missions_and_can_be_cancelled() {
  Rig rig;
  app::AppController c(rig.devices(), 1);
  c.begin(0);
  doCheckIn(c, 3, 3);
  std::string names[3];
  for (int i = 0; i < 3; ++i) names[i] = c.viewModel().missions[i].name;

  press(c, NavInput::kLeftShort);    // 생활 메뉴
  press(c, NavInput::kRightShort);   // 체크인 항목
  press(c, NavInput::kCenterShort);  // 열기
  TEST_ASSERT_EQUAL(Screen::kCheckIn, c.navigator().current());
  doCheckIn(c, 1, 1);
  TEST_ASSERT_EQUAL(Screen::kMain, c.navigator().current());
  for (int i = 0; i < 3; ++i) TEST_ASSERT_TRUE(names[i] == c.viewModel().missions[i].name);

  press(c, NavInput::kLeftShort);
  press(c, NavInput::kRightShort);
  press(c, NavInput::kCenterShort);
  TEST_ASSERT_EQUAL(Screen::kCheckIn, c.navigator().current());
  press(c, NavInput::kCenterLong);  // 취소
  TEST_ASSERT_EQUAL(Screen::kMain, c.navigator().current());
}

void test_first_checkin_cannot_be_cancelled() {
  Rig rig;
  app::AppController c(rig.devices(), 1);
  c.begin(0);
  press(c, NavInput::kCenterLong);
  TEST_ASSERT_EQUAL(Screen::kCheckIn, c.navigator().current());
}

void test_unreadable_or_old_save_is_ignored() {
  Rig rig;
  const uint32_t junk = 0xDEADBEEF;
  rig.storage.save("save", &junk, sizeof(junk));
  app::AppController c(rig.devices(), 1);
  c.begin(0);
  TEST_ASSERT_EQUAL(Screen::kCheckIn, c.navigator().current());
  TEST_ASSERT_EQUAL_UINT32(0, c.viewModel().coins);
}

void test_redraw_flag() {
  Rig rig;
  app::AppController c(rig.devices(), 1);
  c.begin(0);
  TEST_ASSERT_TRUE(c.consumeRedraw());
  TEST_ASSERT_FALSE(c.consumeRedraw());
  c.handleInput(NavInput::kRightShort, 0);
  TEST_ASSERT_TRUE(c.consumeRedraw());

  rig.clock.t.minute = 31;
  c.update(1000);
  TEST_ASSERT_TRUE(c.consumeRedraw());  // 분이 바뀌면 시계를 다시 그립니다.
}

void test_all_catalog_ids_fit_in_save_buffer() {
  for (const auto& mission : maeumgochi::missionCatalog()) {
    TEST_ASSERT_TRUE(strlen(mission.id) < app::kMissionIdSize);
  }
}

}  // namespace

extern "C" void setUp(void) {}
extern "C" void tearDown(void) {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_boot_shows_checkin_screen);
  RUN_TEST(test_checkin_assigns_three_missions_and_opens_main);
  RUN_TEST(test_mission_start_complete_pays_coins_and_vibrates);
  RUN_TEST(test_second_completion_grows_character_with_longer_vibration);
  RUN_TEST(test_progress_is_restored_after_reboot);
  RUN_TEST(test_new_day_requires_checkin_and_does_not_repeat_yesterdays_missions);
  RUN_TEST(test_midnight_rollover_while_running);
  RUN_TEST(test_sunlight_accumulates_and_is_restored_same_day);
  RUN_TEST(test_runs_without_optional_devices);
  RUN_TEST(test_missing_light_sensor_does_not_block_other_screens);
  RUN_TEST(test_recheckin_keeps_todays_missions_and_can_be_cancelled);
  RUN_TEST(test_first_checkin_cannot_be_cancelled);
  RUN_TEST(test_unreadable_or_old_save_is_ignored);
  RUN_TEST(test_redraw_flag);
  RUN_TEST(test_all_catalog_ids_fit_in_save_buffer);
  return UNITY_END();
}
