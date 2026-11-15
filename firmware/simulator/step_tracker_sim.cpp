// StepTracker를 PC에서 직접 만져보는 대화형 시뮬레이터입니다.
// 가짜 센서의 걸음 수와 시간을 명령으로 바꿔 가며 결과를 확인합니다.
//
// 빌드와 실행 (firmware 디렉터리에서):
//   g++ -std=c++17 -Iinclude simulator/step_tracker_sim.cpp src/activity/step_tracker.cpp -o .pio/step_sim.exe
//   .pio/step_sim.exe
//
// Windows 콘솔에서 글자가 깨지지 않도록 화면 출력은 영어로 둡니다.

#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>

#include "activity/step_tracker.h"
#include "hal/motion_sensor.h"

namespace {

struct Sim {
  activity::StepTracker tracker;
  uint32_t nowMs = 0;
  uint32_t sensorCount = 0;  // 가짜 센서의 내장 걸음 수 카운터
  bool connected = true;

  void feed() {
    const hal::MotionReading reading = {0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, sensorCount, connected};
    tracker.update(reading, nowMs);
  }

  void print() const {
    std::cout << "  time=" << nowMs / 1000.0 << "s"
              << "  sensor=" << (connected ? "connected" : "DISCONNECTED")
              << " (raw count " << sensorCount << ")\n"
              << "  -> steps=" << tracker.steps()
              << "  activeSeconds=" << tracker.activeSeconds()
              << "  walking=" << (tracker.isWalking() ? "yes" : "no")
              << "  sensorValid=" << (tracker.isSensorValid() ? "yes" : "no") << "\n";
  }
};

void printHelp() {
  std::cout << "Commands:\n"
               "  w <steps> <seconds>  walk: add steps evenly over the given seconds\n"
               "  s <steps>            add steps instantly (no time passes)\n"
               "  t <seconds>          stand still: let time pass without steps\n"
               "  r                    reset tracker (what a mission start does)\n"
               "  d                    disconnect sensor\n"
               "  c                    reconnect sensor (raw count restarts from 0)\n"
               "  p                    print current state\n"
               "  h                    help\n"
               "  q                    quit\n";
}

}  // namespace

int main() {
  Sim sim;
  sim.feed();  // 첫 값은 기준으로만 쓰입니다.

  std::cout << "=== StepTracker simulator ===\n";
  printHelp();
  sim.print();

  std::string line;
  while (std::cout << "> " && std::getline(std::cin, line)) {
    std::istringstream in(line);
    std::string command;
    if (!(in >> command)) {
      continue;
    }

    if (command == "q") {
      break;
    } else if (command == "h") {
      printHelp();
      continue;
    } else if (command == "p") {
      // 상태만 출력
    } else if (command == "w") {
      uint32_t steps = 0;
      double seconds = 0;
      if (!(in >> steps >> seconds) || steps == 0 || seconds <= 0) {
        std::cout << "  usage: w <steps> <seconds>   e.g. w 100 60\n";
        continue;
      }
      // 걸음마다 시간을 조금씩 흘려 보내며 한 걸음씩 더합니다.
      const double stepIntervalMs = seconds * 1000.0 / steps;
      const uint32_t startMs = sim.nowMs;
      for (uint32_t i = 1; i <= steps; ++i) {
        sim.nowMs = startMs + static_cast<uint32_t>(stepIntervalMs * i);
        if (sim.connected) {
          sim.sensorCount += 1;
        }
        sim.feed();
      }
    } else if (command == "s") {
      uint32_t steps = 0;
      if (!(in >> steps)) {
        std::cout << "  usage: s <steps>   e.g. s 10\n";
        continue;
      }
      if (sim.connected) {
        sim.sensorCount += steps;
      }
      sim.feed();
    } else if (command == "t") {
      double seconds = 0;
      if (!(in >> seconds) || seconds <= 0) {
        std::cout << "  usage: t <seconds>   e.g. t 300\n";
        continue;
      }
      // 실제 루프처럼 100ms마다 읽습니다.
      const uint32_t endMs = sim.nowMs + static_cast<uint32_t>(seconds * 1000.0);
      while (sim.nowMs < endMs) {
        sim.nowMs += (endMs - sim.nowMs < 100) ? endMs - sim.nowMs : 100;
        sim.feed();
      }
    } else if (command == "r") {
      sim.tracker.reset();
      sim.feed();
      std::cout << "  tracker reset (mission start)\n";
    } else if (command == "d") {
      sim.connected = false;
      sim.feed();
    } else if (command == "c") {
      sim.connected = true;
      sim.sensorCount = 0;  // 실제 센서도 전원이 다시 들어오면 0부터 셉니다.
      sim.feed();
    } else {
      std::cout << "  unknown command. type h for help\n";
      continue;
    }

    sim.print();
  }

  return 0;
}
