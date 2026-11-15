// fishing_demo.h - fishing mini game visuals (6 phases). Needs screens_demo.h and the files it includes.
//   maeum_fish::fishScene(tft, maeum_fish::BITE, 0.5f);     // phase, t (0..1: bobber wobble, or marker position in BITE)
// Phases: WAIT (waiting) -> NIBBLE (small bites) -> BITE (shake in the orange zone!) -> PULL (fish jumps) -> CAUGHT (result)
//         MISS (fish got away; no penalty, "try again")
#pragma once
#include "screens_demo.h"

namespace maeum_fish {
using namespace maeum_scene;
using namespace maeum_screen;

enum Phase { WAIT = 0, NIBBLE = 1, BITE = 2, PULL = 3, CAUGHT = 4, MISS = 5 };
const float ZONE_START = 0.40f, ZONE_END = 0.60f;     // shake inside this part of the bar = success

inline void ring(Adafruit_GFX& g, int cx, int cy, int rx, int ry) { ellipse(g, cx, cy, rx, ry, WATER_L); ellipse(g, cx, cy, rx - 2, ry - 1, WATER); }

inline void fish(Adafruit_GFX& g, int cx, int cy, int len, bool faceLeft, uint16_t body, uint16_t belly, bool details) {
  int hd = faceLeft ? -1 : 1, rx = len / 2, ry = len / 4;
  g.fillTriangle(cx - hd * (rx - 2), cy, cx - hd * (rx + len / 4), cy - ry, cx - hd * (rx + len / 4), cy + ry, body);   // tail
  ellipse(g, cx, cy, rx, ry, body);
  if (details) {
    ellipse(g, cx, cy + ry / 3, rx - 3, ry / 2, belly);
    g.fillTriangle(cx - hd * 2, cy - ry + 1, cx - hd * (len / 5), cy - ry - len / 8, cx - hd * (len / 3), cy - ry + 2, ORANGE_D);   // fin
    int er = len / 12 < 2 ? 2 : len / 12;
    g.fillCircle(cx + hd * (rx / 2), cy - ry / 4, er, INK); g.fillCircle(cx + hd * (rx / 2) - 1, cy - ry / 4 - 1, 1, CREAM);
  }
}

inline void star(Adafruit_GFX& g, int x, int y, int big, int small, uint16_t c) {
  g.fillTriangle(x, y - big, x - small, y, x + small, y, c); g.fillTriangle(x, y + big, x - small, y, x + small, y, c);
  g.fillTriangle(x - big, y, x, y - small, x, y + small, c); g.fillTriangle(x + big, y, x, y - small, x, y + small, c);
}

inline void pond(Adafruit_GFX& g) {
  g.fillScreen(WALL);
  g.fillRect(0, 140, 240, 100, WATER);
  const int rx[] = {104, 160, 60, 192}, ry[] = {168, 184, 204, 156}, rw[] = {20, 24, 24, 20};
  for (int i = 0; i < 4; i++) g.fillRoundRect(rx[i], ry[i], rw[i], 3, 1, WATER_L);
  g.fillRect(0, 124, 96, 16, WOOD); g.fillRect(76, 140, 8, 32, WOOD_D);
}

inline void fishScene(Adafruit_GFX& g, int phase, float t) {
  pond(g);
  const int tipX = 140, tipY = 66;
  int bob = 138;
  if (phase == WAIT) bob = 138 + (int)lroundf(2.0f * sinf(t * 6.283f));
  if (phase == NIBBLE) bob = 146 + (int)lroundf(3.0f * sinf(t * 12.566f));
  if (phase == BITE) bob = 156;
  const uint8_t mood = (phase == PULL || phase == CAUGHT) ? maeum::MOOD_HAPPY : maeum::MOOD_NORMAL;
  const int rightArm = (phase == PULL) ? 135 : 125;

  if (phase == CAUGHT) {
    pet(g, 46, 84, mood, maeum::ARM_UP, maeum::ARM_UP);
    fish(g, 156, 84, 70, true, ORANGE_D, CREAM, true);
    star(g, 206, 52, 9, 3, ORANGE); star(g, 112, 56, 6, 2, ORANGE); star(g, 214, 96, 5, 2, ORANGE);
    header(g, "낚시하기", "1 / 3");
    g.fillRoundRect(28, 138, 184, 62, 14, OUTLINE); g.fillRoundRect(31, 141, 178, 56, 12, CREAM);
    textC(g, 120, 144, "잡았어요!", INK, 2);
    coin(g, 104, 188); text(g, 118, 180, "+5", INK);
    hints(g, "", "확인", "");
    return;
  }

  pet(g, 46, 84, mood, maeum::ARM_DOWN, rightArm);
  int hx, hy; maeum::armHand(46, 84, 100, 2, true, rightArm, hx, hy);

  if (phase == PULL) {
    thickLine(g, hx, hy, 150, 46, ROD);
    g.drawLine(150, 46, 114, 98, METAL);
    fish(g, 138, 98, 46, true, ORANGE_D, CREAM, true);
    ring(g, 138, 146, 22, 4);
    g.fillCircle(118, 132, 3, DROP); g.fillCircle(128, 122, 2, DROP); g.fillCircle(158, 126, 3, DROP); g.fillCircle(166, 136, 2, DROP);
  } else {
    thickLine(g, hx, hy, tipX, tipY, ROD);
    g.drawFastVLine(tipX, tipY, bob - tipY, METAL);
    if (phase == WAIT) { ring(g, tipX, 144, 16, 3); fish(g, 196, 196, 30, true, SHADOW, SHADOW, false); }
    if (phase == NIBBLE) { ring(g, tipX, 148, 12, 3); ring(g, tipX, 148, 24, 5); fish(g, 164, 176, 38, true, SHADOW, SHADOW, false); }
    if (phase == BITE) {
      ring(g, tipX, 156, 14, 3); ring(g, tipX, 156, 28, 6); fish(g, 206, 152, 34, true, SHADOW, SHADOW, false);
      g.fillRect(138, 34, 5, 16, ORANGE); g.fillRect(138, 54, 5, 5, ORANGE);                                          // "!"
      sparks(g, 104, 120);
    }
    if (phase == MISS) { ring(g, tipX, 144, 16, 3); fish(g, 208, 196, 30, false, SHADOW, SHADOW, false); }
    g.fillCircle(tipX, bob, 6, ORANGE);
  }

  header(g, "낚시하기", "1 / 3");
  if (phase == WAIT) textC(g, 120, 206, "기다리는 중...", TEXT);
  if (phase == NIBBLE) textC(g, 120, 206, "어? 뭔가 와요...", TEXT);
  if (phase == BITE) {
    textC(g, 120, 168, "지금 흔들어요!", TEXT);
    g.fillRoundRect(32, 196, 176, 12, 6, TRACK);
    g.fillRoundRect(32 + (int)(ZONE_START * 176), 196, (int)((ZONE_END - ZONE_START) * 176), 12, 6, ORANGE);
    int mx = 32 + (int)(t * 176);
    g.fillTriangle(mx - 6, 184, mx + 6, 184, mx, 195, INK);
  }
  if (phase == MISS) { textC(g, 120, 164, "아쉬워요! 다시 해볼까요?", TEXT); hints(g, "", "다시", ""); }
}

}  // namespace maeum_fish
