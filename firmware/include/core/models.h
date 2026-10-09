#pragma once

#include <stdint.h>

// 사용자 기분과 에너지
struct UserCheckIn {
  int mood;       // 1: 매우 나쁨 ~ 5: 매우 좋음
  int energy;     // 1: 매우 낮음 ~ 5: 매우 높음
};

// 센서 및 활동 결과
struct ActivityData {
  int steps;                // 오늘 걸음 수
  unsigned long walkSeconds;
  float distanceM;          // 산책 거리, 미터
  unsigned long sunlightSeconds;
  bool gpsValid;
};

// 캐릭터 상태
struct PetState {
  int level;
  int exp;
  int energy;       // 0~100
  int happiness;    // 0~100
};

// 미션 종류
enum MissionType {
  MISSION_NONE,
  MISSION_WALK,
  MISSION_SUNLIGHT,
  MISSION_MEAL,
  MISSION_WATER,
  MISSION_WASH,
  MISSION_SLEEP,
  MISSION_RECOVERY
};

// 미션 난이도
enum MissionDifficulty {
  DIFFICULTY_EASY,
  DIFFICULTY_NORMAL,
  DIFFICULTY_HARD
};

// 미션 정보
struct Mission {
  MissionType type;
  MissionDifficulty difficulty;

  int target;       // 목표값
  int progress;     // 현재 진행도
  bool completed;
};

// 시간 관련 정보
struct TimeContext {
  int hour;           // 0~23
  int minute;         // 0~59
  int inactiveDays;   // 마지막 사용 이후 경과 일수
};

// 하루 기록
struct DailyRecord {
  int year;
  int month;
  int day;

  int steps;
  int walkMinutes;
  float distanceM;
  int sunlightMinutes;
  int completedMissions;

  int moodBefore;
  int moodAfter;
  int petLevel;
};

// 전체 앱에서 사용할 상태
struct AppState {
  UserCheckIn user;
  ActivityData activity;
  PetState pet;
  Mission currentMission;
  TimeContext time;
  DailyRecord today;
};