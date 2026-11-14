#pragma once

namespace board {
constexpr int kButtonLeft = D0;
constexpr int kButtonCenter = D2;
constexpr int kButtonRight = D9;

constexpr int kLcdSck = D8;
constexpr int kLcdMosi = D10;
constexpr int kLcdRes = D1;
constexpr int kLcdDc = D3;
constexpr int kLcdCs = -1;  // CS를 GND에 고정

constexpr int kHapticGate = D11;
}  // namespace board