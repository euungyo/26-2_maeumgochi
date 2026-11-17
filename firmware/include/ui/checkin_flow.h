#pragma once

#include <stdint.h>

#include "ui/screen_navigator.h"

namespace ui {

constexpr uint8_t kCheckInMin = 1;
constexpr uint8_t kCheckInMax = 5;
constexpr uint8_t kCheckInDefault = 3;

enum class CheckInResult : uint8_t {
  kNone = 0,   // 바뀐 것 없음
  kUpdated,    // 선택값 또는 단계가 바뀜 (화면 갱신 필요)
  kCompleted,  // 기분과 에너지 입력 완료
  kCancelled,  // 첫 단계에서 뒤로 가기 (취소가 허용된 경우만)
};

// 체크인 화면의 2단계 입력(기분 -> 에너지)을 처리합니다.
// 왼쪽 = -, 가운데 = 확정, 오른쪽 = +, 가운데 길게 = 이전 단계(첫 단계에서는 취소).
// ScreenNavigator는 체크인 화면에서 입력을 무시하므로 이 클래스가 입력을 받습니다.
class CheckInFlow {
 public:
  enum class Step : uint8_t { kMood = 0, kEnergy = 1 };

  CheckInFlow() { reset(false); }

  // 처음부터 다시 시작합니다. allowCancel은 이미 오늘 체크인을 한 뒤 다시 여는 경우에 true입니다.
  void reset(bool allowCancel);
  CheckInResult handle(NavInput input);

  Step step() const { return step_; }
  // 지금 선택 중인 값 (1~5)
  uint8_t value() const { return value_; }
  // kCompleted 이후에 읽습니다.
  uint8_t mood() const { return mood_; }
  uint8_t energy() const { return energy_; }

 private:
  Step step_;
  uint8_t value_;
  uint8_t mood_;
  uint8_t energy_;
  bool allowCancel_;
};

}  // namespace ui
