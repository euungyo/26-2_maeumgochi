#include "core/mission_generator.h"

#include <algorithm>
#include <limits>

#include "core/mission_catalog.h"

namespace maeumgochi {
namespace {

std::vector<const MissionDefinition*> filterPreferredDifficulty(
    const std::vector<const MissionDefinition*>& candidates,
    uint8_t targetDifficulty) {
  uint8_t preferredDifficulty = 0;

  for (const auto* mission : candidates) {
    if (mission->difficulty <= targetDifficulty &&
        mission->difficulty > preferredDifficulty) {
      preferredDifficulty = mission->difficulty;
    }
  }

  if (preferredDifficulty == 0) {
    preferredDifficulty = std::numeric_limits<uint8_t>::max();
    for (const auto* mission : candidates) {
      preferredDifficulty = std::min(preferredDifficulty, mission->difficulty);
    }
  }

  std::vector<const MissionDefinition*> preferred;
  for (const auto* mission : candidates) {
    if (mission->difficulty == preferredDifficulty) {
      preferred.push_back(mission);
    }
  }

  return preferred;
}

}  // namespace

std::optional<uint8_t> calculateMissionDifficulty(int mood, int energy) {
  constexpr int kMinimumCheckInValue = 1;
  constexpr int kMaximumCheckInValue = 5;

  if (mood < kMinimumCheckInValue || mood > kMaximumCheckInValue ||
      energy < kMinimumCheckInValue || energy > kMaximumCheckInValue) {
    return std::nullopt;
  }

  return static_cast<uint8_t>((mood + energy + 1) / 2);
}

std::vector<const MissionDefinition*> filterMissionsByPeriod(
    MissionPeriod period) {
  std::vector<const MissionDefinition*> candidates;

  for (const auto& mission : missionCatalog()) {
    if (mission.period == period) {
      candidates.push_back(&mission);
    }
  }

  return candidates;
}

std::vector<const MissionDefinition*> excludePreviousMissions(
    const std::vector<const MissionDefinition*>& candidates,
    const std::vector<std::string_view>& previousMissionIds) {
  std::vector<const MissionDefinition*> available;

  for (const auto* mission : candidates) {
    const auto previous =
        std::find(previousMissionIds.begin(), previousMissionIds.end(), mission->id);
    if (previous == previousMissionIds.end()) {
      available.push_back(mission);
    }
  }

  return available;
}

const MissionDefinition* selectMission(
    MissionPeriod period,
    uint8_t targetDifficulty,
    const std::vector<std::string_view>& previousMissionIds,
    std::mt19937& random) {
  const auto periodCandidates = filterMissionsByPeriod(period);
  const auto available =
      excludePreviousMissions(periodCandidates, previousMissionIds);
  if (available.empty()) {
    return nullptr;
  }

  const auto preferred =
      filterPreferredDifficulty(available, targetDifficulty);
  std::uniform_int_distribution<std::size_t> distribution(
      0, preferred.size() - 1);
  return preferred[distribution(random)];
}

std::optional<DailyMissionAssignment> assignDailyMissions(
    int mood,
    int energy,
    const std::vector<std::string_view>& previousMissionIds,
    std::mt19937& random) {
  const auto difficulty = calculateMissionDifficulty(mood, energy);
  if (!difficulty.has_value()) {
    return std::nullopt;
  }

  DailyMissionAssignment assignment{
      *difficulty,
      {
          selectMission(MissionPeriod::Morning, *difficulty,
                        previousMissionIds, random),
          selectMission(MissionPeriod::Afternoon, *difficulty,
                        previousMissionIds, random),
          selectMission(MissionPeriod::Special, *difficulty,
                        previousMissionIds, random),
      },
  };

  const auto missing = std::find(assignment.missions.begin(),
                                 assignment.missions.end(), nullptr);
  if (missing != assignment.missions.end()) {
    return std::nullopt;
  }

  return assignment;
}

}  // namespace maeumgochi
