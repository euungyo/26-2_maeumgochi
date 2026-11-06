#pragma once

#include <optional>
#include <stdint.h>
#include <string_view>
#include <vector>

#include "core/mission_models.h"

namespace maeumgochi {

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

}  // namespace maeumgochi
