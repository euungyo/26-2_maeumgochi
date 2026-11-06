#pragma once

#include <array>
#include <optional>
#include <random>
#include <stdint.h>
#include <string_view>
#include <vector>

#include "core/mission_models.h"

namespace maeumgochi {

struct DailyMissionAssignment {
  uint8_t targetDifficulty;
  std::array<const MissionDefinition*, 3> missions;
};

// mood와 energy가 모두 1~5일 때 목표 난이도(1~5)를 반환한다.
// 한 값이라도 1이면 무리한 미션을 피하기 위해 난이도 1을 반환하고,
// 그 외에는 두 값의 평균을 0.5부터 올림한다.
std::optional<uint8_t> calculateMissionDifficulty(int mood, int energy);

// 전체 카탈로그에서 지정한 시간대의 미션만 추린다.
std::vector<const MissionDefinition*> filterMissionsByPeriod(
    MissionPeriod period);

// 전날 배정된 ID와 일치하는 미션을 후보에서 제외한다.
std::vector<const MissionDefinition*> excludePreviousMissions(
    const std::vector<const MissionDefinition*>& candidates,
    const std::vector<std::string_view>& previousMissionIds);

// 시간대, 전날 기록과 난이도를 적용한 후보 중 하나를 균등 무작위로 뽑는다.
// 같은 난이도가 없으면 더 낮은 난이도를 먼저 사용하고, 낮은 후보도 없을
// 때만 가장 가까운 높은 난이도를 사용한다.
const MissionDefinition* selectMission(
    MissionPeriod period,
    uint8_t targetDifficulty,
    const std::vector<std::string_view>& previousMissionIds,
    std::mt19937& random);

// 오전, 오후, 특수 시간대에서 각각 한 개씩 오늘의 미션을 배정한다.
std::optional<DailyMissionAssignment> assignDailyMissions(
    int mood,
    int energy,
    const std::vector<std::string_view>& previousMissionIds,
    std::mt19937& random);

}  // namespace maeumgochi
