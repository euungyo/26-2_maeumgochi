#include <unity.h>

#include <cstring>

#include "core/mission_catalog.h"
#include "core/mission_generator.h"

namespace {

void test_catalog_contains_all_planned_missions() {
  const auto& catalog = maeumgochi::missionCatalog();

  TEST_ASSERT_EQUAL_UINT32(13, catalog.size());

  uint8_t morningCount = 0;
  uint8_t afternoonCount = 0;
  uint8_t specialCount = 0;
  bool hasSensorMission = false;

  for (const auto& mission : catalog) {
    TEST_ASSERT_NOT_NULL(mission.id);
    TEST_ASSERT_NOT_NULL(mission.name);
    TEST_ASSERT_TRUE(mission.difficulty >= 1 && mission.difficulty <= 5);

    morningCount += mission.period == maeumgochi::MissionPeriod::Morning;
    afternoonCount += mission.period == maeumgochi::MissionPeriod::Afternoon;
    specialCount += mission.period == maeumgochi::MissionPeriod::Special;
    hasSensorMission |=
        mission.completionMethod == maeumgochi::CompletionMethod::Sensor;
  }

  TEST_ASSERT_EQUAL_UINT8(5, morningCount);
  TEST_ASSERT_EQUAL_UINT8(6, afternoonCount);
  TEST_ASSERT_EQUAL_UINT8(2, specialCount);
  TEST_ASSERT_TRUE(hasSensorMission);
}

void test_catalog_ids_are_unique() {
  const auto& catalog = maeumgochi::missionCatalog();

  for (size_t left = 0; left < catalog.size(); ++left) {
    for (size_t right = left + 1; right < catalog.size(); ++right) {
      TEST_ASSERT_NOT_EQUAL(0, std::strcmp(catalog[left].id, catalog[right].id));
    }
  }
}

void test_difficulty_protects_lowest_check_in() {
  const auto difficulty = maeumgochi::calculateMissionDifficulty(1, 5);

  TEST_ASSERT_TRUE(difficulty.has_value());
  TEST_ASSERT_EQUAL_UINT8(1, *difficulty);
}

void test_difficulty_rounds_average_up() {
  TEST_ASSERT_EQUAL_UINT8(3, *maeumgochi::calculateMissionDifficulty(2, 3));
  TEST_ASSERT_EQUAL_UINT8(5, *maeumgochi::calculateMissionDifficulty(5, 4));
}

void test_difficulty_rejects_out_of_range_check_in() {
  TEST_ASSERT_FALSE(maeumgochi::calculateMissionDifficulty(0, 3).has_value());
  TEST_ASSERT_FALSE(maeumgochi::calculateMissionDifficulty(3, 6).has_value());
}

void test_candidates_are_filtered_by_period() {
  using maeumgochi::MissionPeriod;

  const auto morning = maeumgochi::filterMissionsByPeriod(MissionPeriod::Morning);
  const auto afternoon =
      maeumgochi::filterMissionsByPeriod(MissionPeriod::Afternoon);
  const auto special = maeumgochi::filterMissionsByPeriod(MissionPeriod::Special);

  TEST_ASSERT_EQUAL_UINT32(5, morning.size());
  TEST_ASSERT_EQUAL_UINT32(6, afternoon.size());
  TEST_ASSERT_EQUAL_UINT32(2, special.size());

  for (const auto* mission : morning) {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(MissionPeriod::Morning),
                          static_cast<int>(mission->period));
  }
  for (const auto* mission : afternoon) {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(MissionPeriod::Afternoon),
                          static_cast<int>(mission->period));
  }
  for (const auto* mission : special) {
    TEST_ASSERT_EQUAL_INT(static_cast<int>(MissionPeriod::Special),
                          static_cast<int>(mission->period));
  }
}

void test_previous_day_missions_are_excluded() {
  using maeumgochi::MissionPeriod;

  const auto afternoon =
      maeumgochi::filterMissionsByPeriod(MissionPeriod::Afternoon);
  const auto available = maeumgochi::excludePreviousMissions(
      afternoon, {"shower", "walk", "unknown_mission"});

  TEST_ASSERT_EQUAL_UINT32(4, available.size());
  for (const auto* mission : available) {
    TEST_ASSERT_NOT_EQUAL(0, std::strcmp("shower", mission->id));
    TEST_ASSERT_NOT_EQUAL(0, std::strcmp("walk", mission->id));
  }
}

}  // namespace

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_catalog_contains_all_planned_missions);
  RUN_TEST(test_catalog_ids_are_unique);
  RUN_TEST(test_difficulty_protects_lowest_check_in);
  RUN_TEST(test_difficulty_rounds_average_up);
  RUN_TEST(test_difficulty_rejects_out_of_range_check_in);
  RUN_TEST(test_candidates_are_filtered_by_period);
  RUN_TEST(test_previous_day_missions_are_excluded);
  return UNITY_END();
}
