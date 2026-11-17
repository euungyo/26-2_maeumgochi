#pragma once

#include <stdint.h>

#include "ui/screen_navigator.h"

namespace ui {

enum class MissionViewState : uint8_t { kNotStarted = 0, kInProgress, kCompleted };
enum class MissionViewPeriod : uint8_t { kMorning = 0, kAfternoon, kSpecial };

struct MissionView {
  const char* name = "";  // UTF-8, 카탈로그 문자열을 그대로 가리킵니다.
  MissionViewPeriod period = MissionViewPeriod::kMorning;
  MissionViewState state = MissionViewState::kNotStarted;
  uint16_t reward = 0;       // 완료 시 코인
  bool sensorBased = false;  // 센서로 확인하는 미션인지
};

// 화면이 그리는 데이터만 모은 구조체입니다. Adafruit나 core 구현에 의존하지 않습니다.
struct ScreenViewModel {
  bool timeValid = false;
  uint8_t hour = 0;
  uint8_t minute = 0;

  uint32_t coins = 0;
  uint8_t characterStage = 1;  // 1~3
  uint8_t nextGrowthIn = 0;    // 다음 성장까지 남은 미션 수 (최종 단계면 0)
  uint8_t missionsPerGrowth = 2;

  uint8_t checkInStep = 0;   // 0 기분, 1 에너지
  uint8_t checkInValue = 3;  // 1~5

  uint8_t missionCount = 0;  // 0(아직 배정 전) 또는 kMissionSlotCount
  uint8_t currentMission = 0;   // 메인 화면에 보일 미션
  uint8_t selectedMission = 0;  // 상세 화면에 보일 미션
  MissionView missions[kMissionSlotCount];
};

}  // namespace ui
