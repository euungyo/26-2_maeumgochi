#pragma once

#include <stddef.h>
#include <stdint.h>
#include <type_traits>

namespace app {

// 구조를 바꾸면 kSaveVersion을 올립니다. hal::Storage::load()는 크기가 다르면 false를 반환하므로
// 크기가 바뀌는 변경은 이전 구조체를 읽어 옮기는 코드가 필요합니다.
constexpr uint16_t kSaveVersion = 1;
constexpr uint8_t kSaveMissionSlots = 3;
// 카탈로그 순서가 바뀌어도 복원되도록 미션은 번호가 아니라 id 문자열로 저장합니다.
constexpr size_t kMissionIdSize = 16;

struct SaveData {
  uint16_t version;
  uint8_t characterStage;
  uint8_t mood;           // 0이면 오늘 체크인 안 함
  uint8_t energy;
  uint8_t missionCount;   // 0 또는 kSaveMissionSlots
  uint8_t previousCount;  // 전날 미션 개수
  uint8_t reserved;
  uint32_t coins;
  uint32_t completedMissions;
  uint32_t dateKey;         // yyyymmdd, 0이면 시계를 읽지 못함
  uint32_t sunlightSeconds;  // dateKey 날짜의 누적 햇빛 시간
  char missionId[kSaveMissionSlots][kMissionIdSize];
  uint8_t missionStatus[kSaveMissionSlots];  // core MissionStatus 값
  char previousId[kSaveMissionSlots][kMissionIdSize];
};

static_assert(std::is_trivially_copyable<SaveData>::value, "SaveData는 바이트로 저장됩니다");

}  // namespace app
