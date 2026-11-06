#include <unity.h>

#include <cstring>

#include "core/mission_catalog.h"

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

}  // namespace

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_catalog_contains_all_planned_missions);
  RUN_TEST(test_catalog_ids_are_unique);
  return UNITY_END();
}
