#include "ui/checkin_flow.h"

namespace ui {

void CheckInFlow::reset(bool allowCancel) {
  step_ = Step::kMood;
  value_ = kCheckInDefault;
  mood_ = 0;
  energy_ = 0;
  allowCancel_ = allowCancel;
}

CheckInResult CheckInFlow::handle(NavInput input) {
  switch (input) {
    case NavInput::kLeftShort:
      if (value_ <= kCheckInMin) return CheckInResult::kNone;
      --value_;
      return CheckInResult::kUpdated;

    case NavInput::kRightShort:
      if (value_ >= kCheckInMax) return CheckInResult::kNone;
      ++value_;
      return CheckInResult::kUpdated;

    case NavInput::kCenterShort:
      if (step_ == Step::kMood) {
        mood_ = value_;
        step_ = Step::kEnergy;
        value_ = kCheckInDefault;
        return CheckInResult::kUpdated;
      }
      energy_ = value_;
      return CheckInResult::kCompleted;

    case NavInput::kCenterLong:
      if (step_ == Step::kEnergy) {
        step_ = Step::kMood;
        value_ = mood_;  // 방금 고른 기분 값에서 다시 시작합니다.
        return CheckInResult::kUpdated;
      }
      return allowCancel_ ? CheckInResult::kCancelled : CheckInResult::kNone;
  }
  return CheckInResult::kNone;
}

}  // namespace ui
