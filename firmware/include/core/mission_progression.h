#pragma once

#include <stdint.h>

#include "core/mission_models.h"

namespace maeumgochi {

constexpr uint8_t kCharacterStageCount = 3;
constexpr uint8_t kMissionsPerCharacterGrowth = 2;

enum class MissionStatus : uint8_t {
  NotStarted,
  InProgress,
  Completed,
};

// 오늘 배정된 미션의 실행 상태입니다.
struct MissionProgress {
  const MissionDefinition* definition = nullptr;
  MissionStatus status = MissionStatus::NotStarted;
  bool rewardGranted = false;
};

// 미션 완료로 누적되는 플레이어 성장 상태입니다.
struct PlayerProgress {
  uint32_t coins = 0;
  uint32_t completedMissions = 0;
  uint8_t characterStage = 1;
};

struct MissionCompletionResult {
  bool completed = false;
  uint16_t coinsAwarded = 0;
  bool characterGrew = false;
  uint8_t characterStage = 1;
};

MissionProgress createMissionProgress(const MissionDefinition& definition);

// 시작 전 미션만 진행 중 상태로 바꾼다.
bool startMission(MissionProgress& mission);

// 난이도 1~5를 각각 100~500코인으로 변환한다. 유효하지 않으면 0이다.
uint16_t coinsForDifficulty(uint8_t difficulty);

// 완료 미션 두 개마다 성장하며, 캐릭터 단계는 3단계를 넘지 않는다.
uint8_t characterStageForCompletedMissions(uint32_t completedMissions);

// 진행 중 미션을 완료하고 코인과 캐릭터 성장을 한 번만 반영한다.
MissionCompletionResult completeMission(MissionProgress& mission,
                                        PlayerProgress& player);

}  // namespace maeumgochi
