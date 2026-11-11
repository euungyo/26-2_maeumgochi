#include "core/mission_progression.h"

#include <algorithm>

namespace maeumgochi {

MissionProgress createMissionProgress(const MissionDefinition& definition) {
  return MissionProgress{&definition, MissionStatus::NotStarted, false};
}

bool startMission(MissionProgress& mission) {
  if (mission.definition == nullptr ||
      mission.status != MissionStatus::NotStarted) {
    return false;
  }

  mission.status = MissionStatus::InProgress;
  return true;
}

uint16_t coinsForDifficulty(uint8_t difficulty) {
  constexpr uint8_t kMinimumDifficulty = 1;
  constexpr uint8_t kMaximumDifficulty = 5;

  if (difficulty < kMinimumDifficulty || difficulty > kMaximumDifficulty) {
    return 0;
  }

  return static_cast<uint16_t>(difficulty) * 100;
}

uint8_t characterStageForCompletedMissions(uint32_t completedMissions) {
  const uint32_t growthCount =
      completedMissions / kMissionsPerCharacterGrowth;
  const uint32_t stage = 1 + growthCount;
  return static_cast<uint8_t>(
      std::min<uint32_t>(stage, kCharacterStageCount));
}

MissionCompletionResult completeMission(MissionProgress& mission,
                                        PlayerProgress& player) {
  MissionCompletionResult result{
      false,
      0,
      false,
      player.characterStage,
  };

  if (mission.definition == nullptr ||
      mission.status != MissionStatus::InProgress || mission.rewardGranted) {
    return result;
  }

  const uint16_t reward = coinsForDifficulty(mission.definition->difficulty);
  if (reward == 0) {
    return result;
  }

  const uint8_t previousStage = player.characterStage;

  mission.status = MissionStatus::Completed;
  mission.rewardGranted = true;
  player.coins += reward;
  ++player.completedMissions;
  player.characterStage =
      characterStageForCompletedMissions(player.completedMissions);

  result.completed = true;
  result.coinsAwarded = reward;
  result.characterGrew = player.characterStage > previousStage;
  result.characterStage = player.characterStage;
  return result;
}

}  // namespace maeumgochi
