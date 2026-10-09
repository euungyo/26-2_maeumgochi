#pragma once

#include "models.h"

inline AppState createNormalTestData() {
  AppState state;

  state.user.mood = 3;
  state.user.energy = 3;

  state.activity.steps = 3200;
  state.activity.walkSeconds = 1500;
  state.activity.distanceM = 1720.5;
  state.activity.sunlightSeconds = 900;
  state.activity.gpsValid = true;

  state.pet.level = 1;
  state.pet.exp = 40;
  state.pet.energy = 70;
  state.pet.happiness = 65;

  state.currentMission.type = MISSION_WALK;
  state.currentMission.difficulty = DIFFICULTY_NORMAL;
  state.currentMission.target = 3000;
  state.currentMission.progress = 3200;
  state.currentMission.completed = false;

  state.time.hour = 15;
  state.time.minute = 30;
  state.time.inactiveDays = 0;

  state.today.year = 2026;
  state.today.month = 10;
  state.today.day = 9;
  state.today.steps = 3200;
  state.today.walkMinutes = 25;
  state.today.distanceM = 1720.5;
  state.today.sunlightMinutes = 15;
  state.today.completedMissions = 1;
  state.today.moodBefore = 2;
  state.today.moodAfter = 4;
  state.today.petLevel = 1;

  return state;
}