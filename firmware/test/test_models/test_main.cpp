#include <unity.h>

#include "core/models.h"

namespace {
void test_app_state_can_be_initialized() {
  AppState state{};

  TEST_ASSERT_EQUAL_INT(0, state.activity.steps);
  TEST_ASSERT_EQUAL_INT(0, state.pet.exp);
  TEST_ASSERT_FALSE(state.activity.gpsValid);
  TEST_ASSERT_FALSE(state.currentMission.completed);
}

void test_mission_enums_are_distinct() {
  TEST_ASSERT_NOT_EQUAL(MISSION_NONE, MISSION_WALK);
  TEST_ASSERT_NOT_EQUAL(MISSION_WALK, MISSION_SUNLIGHT);
  TEST_ASSERT_NOT_EQUAL(DIFFICULTY_EASY, DIFFICULTY_NORMAL);
}
}  // namespace

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_app_state_can_be_initialized);
  RUN_TEST(test_mission_enums_are_distinct);
  return UNITY_END();
}
