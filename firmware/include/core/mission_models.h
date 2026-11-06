#pragma once

#include <stdint.h>

namespace maeumgochi {

enum class MissionPeriod : uint8_t {
  Morning,
  Afternoon,
  Special,
};

enum class CompletionMethod : uint8_t {
  Check,
  Sensor,
};

struct MissionDefinition {
  const char* id;
  const char* name;
  MissionPeriod period;
  uint8_t difficulty;
  CompletionMethod completionMethod;
};

}  // namespace maeumgochi
