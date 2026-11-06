#include "core/mission_catalog.h"

namespace maeumgochi {

const std::vector<MissionDefinition>& missionCatalog() {
  static const std::vector<MissionDefinition> missions = {
      {"make_bed", "침대 정리하기", MissionPeriod::Morning, 1,
       CompletionMethod::Check},
      {"drink_water", "물 마시기", MissionPeriod::Morning, 1,
       CompletionMethod::Check},
      {"wash_or_brush", "세수/양치하기", MissionPeriod::Morning, 1,
       CompletionMethod::Check},
      {"stretch", "스트레칭", MissionPeriod::Morning, 1,
       CompletionMethod::Check},
      {"ventilate", "환기하기", MissionPeriod::Morning, 1,
       CompletionMethod::Check},

      {"study", "공부하기", MissionPeriod::Afternoon, 2,
       CompletionMethod::Check},
      {"shower", "샤워하기", MissionPeriod::Afternoon, 3,
       CompletionMethod::Check},
      {"cook", "요리하기", MissionPeriod::Afternoon, 3,
       CompletionMethod::Check},
      {"clean_room", "방 청소하기", MissionPeriod::Afternoon, 4,
       CompletionMethod::Check},
      {"walk", "산책하기", MissionPeriod::Afternoon, 5,
       CompletionMethod::Sensor},
      {"grocery", "장보러 가기", MissionPeriod::Afternoon, 5,
       CompletionMethod::Sensor},

      {"fishing", "낚시하기", MissionPeriod::Special, 1,
       CompletionMethod::Sensor},
      {"water_plant", "식물 물주기", MissionPeriod::Special, 2,
       CompletionMethod::Check},
  };

  return missions;
}

}  // namespace maeumgochi
