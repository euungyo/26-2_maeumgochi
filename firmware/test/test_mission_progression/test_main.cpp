#include <unity.h>

#include "core/mission_progression.h"

namespace {

maeumgochi::MissionDefinition missionWithDifficulty(uint8_t difficulty) {
  return maeumgochi::MissionDefinition{
      "manual_mission",
      "사용자 입력 미션",
      maeumgochi::MissionPeriod::Morning,
      difficulty,
      maeumgochi::CompletionMethod::Check,
  };
}

void test_new_mission_starts_in_not_started_state() {
  const auto definition = missionWithDifficulty(1);
  const auto mission = maeumgochi::createMissionProgress(definition);

  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(maeumgochi::MissionStatus::NotStarted),
      static_cast<int>(mission.status));
  TEST_ASSERT_FALSE(mission.rewardGranted);
}

void test_user_can_start_a_not_started_mission_once() {
  const auto definition = missionWithDifficulty(2);
  auto mission = maeumgochi::createMissionProgress(definition);

  TEST_ASSERT_TRUE(maeumgochi::startMission(mission));
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(maeumgochi::MissionStatus::InProgress),
      static_cast<int>(mission.status));
  TEST_ASSERT_FALSE(maeumgochi::startMission(mission));
}

void test_not_started_mission_cannot_be_completed() {
  const auto definition = missionWithDifficulty(3);
  auto mission = maeumgochi::createMissionProgress(definition);
  maeumgochi::PlayerProgress player;

  const auto result = maeumgochi::completeMission(mission, player);

  TEST_ASSERT_FALSE(result.completed);
  TEST_ASSERT_EQUAL_UINT32(0, player.coins);
  TEST_ASSERT_EQUAL_UINT32(0, player.completedMissions);
}

void test_difficulty_maps_to_one_hundred_coins_per_level() {
  TEST_ASSERT_EQUAL_UINT32(100, maeumgochi::coinsForDifficulty(1));
  TEST_ASSERT_EQUAL_UINT32(200, maeumgochi::coinsForDifficulty(2));
  TEST_ASSERT_EQUAL_UINT32(300, maeumgochi::coinsForDifficulty(3));
  TEST_ASSERT_EQUAL_UINT32(400, maeumgochi::coinsForDifficulty(4));
  TEST_ASSERT_EQUAL_UINT32(500, maeumgochi::coinsForDifficulty(5));
  TEST_ASSERT_EQUAL_UINT32(0, maeumgochi::coinsForDifficulty(0));
  TEST_ASSERT_EQUAL_UINT32(0, maeumgochi::coinsForDifficulty(6));
}

void test_completed_mission_grants_its_difficulty_reward() {
  const auto definition = missionWithDifficulty(4);
  auto mission = maeumgochi::createMissionProgress(definition);
  maeumgochi::PlayerProgress player;
  maeumgochi::startMission(mission);

  const auto result = maeumgochi::completeMission(mission, player);

  TEST_ASSERT_TRUE(result.completed);
  TEST_ASSERT_EQUAL_UINT32(400, result.coinsAwarded);
  TEST_ASSERT_EQUAL_UINT32(400, player.coins);
  TEST_ASSERT_EQUAL_UINT32(1, player.completedMissions);
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(maeumgochi::MissionStatus::Completed),
      static_cast<int>(mission.status));
  TEST_ASSERT_TRUE(mission.rewardGranted);
}

void test_completed_mission_cannot_grant_reward_twice() {
  const auto definition = missionWithDifficulty(5);
  auto mission = maeumgochi::createMissionProgress(definition);
  maeumgochi::PlayerProgress player;
  maeumgochi::startMission(mission);

  const auto first = maeumgochi::completeMission(mission, player);
  const auto second = maeumgochi::completeMission(mission, player);

  TEST_ASSERT_TRUE(first.completed);
  TEST_ASSERT_FALSE(second.completed);
  TEST_ASSERT_EQUAL_UINT32(0, second.coinsAwarded);
  TEST_ASSERT_EQUAL_UINT32(500, player.coins);
  TEST_ASSERT_EQUAL_UINT32(1, player.completedMissions);
}

void test_character_grows_after_every_two_completions_up_to_stage_three() {
  maeumgochi::PlayerProgress player;

  for (uint8_t completion = 1; completion <= 6; ++completion) {
    const auto definition = missionWithDifficulty(1);
    auto mission = maeumgochi::createMissionProgress(definition);
    maeumgochi::startMission(mission);

    const auto result = maeumgochi::completeMission(mission, player);
    TEST_ASSERT_TRUE(result.completed);

    const uint8_t expectedStage =
        completion < 2 ? 1 : (completion < 4 ? 2 : 3);
    TEST_ASSERT_EQUAL_UINT8(expectedStage, player.characterStage);
    TEST_ASSERT_EQUAL(result.characterGrew,
                      completion == 2 || completion == 4);
  }

  TEST_ASSERT_EQUAL_UINT32(6, player.completedMissions);
  TEST_ASSERT_EQUAL_UINT8(3, player.characterStage);
}

void test_invalid_difficulty_does_not_complete_or_reward() {
  const auto definition = missionWithDifficulty(0);
  auto mission = maeumgochi::createMissionProgress(definition);
  maeumgochi::PlayerProgress player;
  maeumgochi::startMission(mission);

  const auto result = maeumgochi::completeMission(mission, player);

  TEST_ASSERT_FALSE(result.completed);
  TEST_ASSERT_EQUAL_INT(
      static_cast<int>(maeumgochi::MissionStatus::InProgress),
      static_cast<int>(mission.status));
  TEST_ASSERT_FALSE(mission.rewardGranted);
  TEST_ASSERT_EQUAL_UINT32(0, player.coins);
  TEST_ASSERT_EQUAL_UINT32(0, player.completedMissions);
}

}  // namespace

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_new_mission_starts_in_not_started_state);
  RUN_TEST(test_user_can_start_a_not_started_mission_once);
  RUN_TEST(test_not_started_mission_cannot_be_completed);
  RUN_TEST(test_difficulty_maps_to_one_hundred_coins_per_level);
  RUN_TEST(test_completed_mission_grants_its_difficulty_reward);
  RUN_TEST(test_completed_mission_cannot_grant_reward_twice);
  RUN_TEST(test_character_grows_after_every_two_completions_up_to_stage_three);
  RUN_TEST(test_invalid_difficulty_does_not_complete_or_reward);
  return UNITY_END();
}
