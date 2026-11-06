#pragma once

#include <vector>

#include "core/mission_models.h"

namespace maeumgochi {

// 기획에서 확정한 일일 배정 대상 미션 13개를 반환한다.
// 식사와 수면은 반복 루틴이므로 이 카탈로그에 포함하지 않는다.
const std::vector<MissionDefinition>& missionCatalog();

}  // namespace maeumgochi
