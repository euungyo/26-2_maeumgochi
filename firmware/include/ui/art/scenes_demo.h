// scenes_demo.h - menu / room / mission scenes drawn with Adafruit_GFX primitives, using maeum::drawCloud for the pet.
// Screen size 240 x 240. Text uses the default Adafruit font (English only). Put it in the same folder as cloud_sprite.h.
#pragma once
#include <Adafruit_GFX.h>
#include <string.h>
#include <math.h>
#include "cloud_sprite.h"
#include "ko_text.h"

namespace maeum_scene {

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) { return maeum::rgb565(r, g, b); }

const uint16_t INK = maeum::C_NAVY, CREAM = maeum::C_CREAM, ORANGE = maeum::C_ORANGE, OUTLINE = maeum::C_OUTLINE;
const uint16_t WOOD = rgb(0xB8, 0x89, 0x5C), WOOD_D = rgb(0x8F, 0x6A, 0x44), GREEN = rgb(0x7F, 0xA5, 0x8B);
const uint16_t PILLOW = rgb(0xE4, 0xDE, 0xCF), ORANGE_D = rgb(0xD9, 0x8B, 0x4A), ROD = rgb(0xD9, 0xB9, 0x8A);

#if MAEUM_THEME == 1   // ---- Soft day (light) ----
const uint16_t BAR = rgb(0x26, 0x39, 0x5A), BAR_TEXT = rgb(0xDD, 0xE8, 0xF5);               // header bar
const uint16_t TEXT = rgb(0x26, 0x39, 0x5A);                                                 // text on scene backgrounds
const uint16_t WALL = rgb(0xC9, 0xDD, 0xEF), FLOOR = rgb(0xEB, 0xDC, 0xC6), LINE = rgb(0x8F, 0xA6, 0xC6);
const uint16_t TILE = rgb(0xB7, 0xCC, 0xE2), SKY = rgb(0xB5, 0xD8, 0xF0), CELESTIAL = ORANGE;
const uint16_t WATER = rgb(0x86, 0xA7, 0xCE), WATER_L = rgb(0xBF, 0xD8, 0xEE), SHADOW = rgb(0x5F, 0x7F, 0xA8), DROP = rgb(0x5E, 0x9B, 0xD0);
const uint16_t HILL = rgb(0xB3, 0xCB, 0xE3), PATH = rgb(0xDC, 0xCB, 0xB0), RUG = rgb(0xDC, 0xAA, 0x8C);
const uint16_t METAL = rgb(0x8F, 0xA3, 0xC0), STEAM = rgb(0xFF, 0xFF, 0xFF), HOUSE_WALL = rgb(0x9F, 0xB3, 0xC8);
const uint16_t MENU_BG = rgb(0xDC, 0xE9, 0xF5), TRACK = rgb(0xB9, 0xC9, 0xDE), SLOT = rgb(0x7F, 0x98, 0xBE);
const uint16_t PILL_OFF_BG = rgb(0xE8, 0xF0, 0xF8), PILL_OFF_BORDER = rgb(0xA9, 0xBB, 0xD4), PILL_OFF_TEXT = rgb(0x5E, 0x73, 0x99);
const uint16_t SEL_BG = rgb(0xFF, 0xE9, 0xD0), SUB_TEXT = rgb(0x6B, 0x7F, 0xA3);
#else                  // ---- Deep navy (night) ----
const uint16_t BAR = rgb(0x12, 0x26, 0x3B), BAR_TEXT = rgb(0x9F, 0xB3, 0xC8);
const uint16_t TEXT = CREAM;
const uint16_t WALL = rgb(0x1B, 0x33, 0x50), FLOOR = rgb(0x24, 0x3F, 0x5C), LINE = rgb(0x2E, 0x45, 0x60);
const uint16_t TILE = LINE, SKY = rgb(0x0E, 0x20, 0x36), CELESTIAL = CREAM;
const uint16_t WATER = LINE, WATER_L = rgb(0x3C, 0x6E, 0x96), SHADOW = WALL, DROP = rgb(0x3C, 0x6E, 0x96);
const uint16_t HILL = LINE, PATH = LINE, RUG = rgb(138, 115, 98);
const uint16_t METAL = rgb(0x9F, 0xB3, 0xC8), STEAM = PILLOW, HOUSE_WALL = METAL;
const uint16_t MENU_BG = rgb(0x12, 0x26, 0x3B), TRACK = rgb(0x5B, 0x78, 0x98), SLOT = METAL;
const uint16_t PILL_OFF_BG = rgb(0x12, 0x26, 0x3B), PILL_OFF_BORDER = LINE, PILL_OFF_TEXT = METAL;
const uint16_t SEL_BG = rgb(0xF6, 0xE3, 0xC8), SUB_TEXT = rgb(0x5E, 0x73, 0x99);
#endif

// ---------- helpers ----------
inline void ellipse(Adafruit_GFX& g, int cx, int cy, int rx, int ry, uint16_t c) {
  for (int dy = -ry; dy <= ry; dy++) {
    float t = 1.0f - (float)(dy * dy) / (float)(ry * ry);
    int w = (int)(rx * sqrtf(t));
    g.drawFastHLine(cx - w, cy + dy, 2 * w + 1, c);
  }
}
inline void thickLine(Adafruit_GFX& g, int x0, int y0, int x1, int y1, uint16_t c) {   // 3 px wide line
  g.drawLine(x0, y0, x1, y1, c); g.drawLine(x0 + 1, y0, x1 + 1, y1, c); g.drawLine(x0, y0 + 1, x1, y1 + 1, c);
}
inline void dashedRect(Adafruit_GFX& g, int x, int y, int w, int h, uint16_t c) {
  for (int i = 0; i < w; i += 8) { g.drawFastHLine(x + i, y, 4, c); g.drawFastHLine(x + i, y + h, 4, c); }
  for (int j = 0; j < h; j += 8) { g.drawFastVLine(x, y + j, 4, c); g.drawFastVLine(x + w, y + j, 4, c); }
}
inline void text(Adafruit_GFX& g, int x, int y, const char* t, uint16_t c, int scale = 1) { maeum::ko::draw(g, x, y, t, c, scale); }
inline void textC(Adafruit_GFX& g, int cx, int y, const char* t, uint16_t c, int scale = 1) { maeum::ko::drawCentered(g, cx, y, t, c, scale); }
inline void textR(Adafruit_GFX& g, int rx, int y, const char* t, uint16_t c, int scale = 1) { maeum::ko::drawRight(g, rx, y, t, c, scale); }
inline void coin(Adafruit_GFX& g, int cx, int cy, int r = 8) { g.fillCircle(cx, cy, r, ORANGE_D); g.fillCircle(cx, cy, r - 2, ORANGE); g.fillRect(cx - 1, cy - r / 2, 2, r, ORANGE_D); }
inline void header(Adafruit_GFX& g, const char* left, const char* right) {
  g.fillRect(0, 0, 240, 26, BAR);
  text(g, 8, 5, left, BAR_TEXT);
  if (right) textR(g, 232, 5, right, BAR_TEXT);
}
inline void headerCoins(Adafruit_GFX& g, const char* left, const char* coins) {       // left title, coin icon + amount on the right
  g.fillRect(0, 0, 240, 26, BAR);
  text(g, 8, 5, left, BAR_TEXT);
  textR(g, 232, 5, coins, BAR_TEXT);
  coin(g, 232 - maeum::ko::width(coins) - 14, 13);
}
inline void pill(Adafruit_GFX& g, int x, int y, int w, const char* label, bool on) {
  if (on) g.fillRoundRect(x, y, w, 24, 6, ORANGE);
  else { g.fillRoundRect(x, y, w, 24, 6, PILL_OFF_BORDER); g.fillRoundRect(x + 2, y + 2, w - 4, 20, 5, PILL_OFF_BG); }
  textC(g, x + w / 2, y + 4, label, on ? INK : PILL_OFF_TEXT);
}
inline void hints(Adafruit_GFX& g, const char* l, const char* c, const char* r) {     // 3 button hint bar (left / center / right)
  if (l[0]) pill(g, 12, 208, 68, l, false);
  if (c[0]) pill(g, 86, 208, 68, c, true);
  if (r[0]) pill(g, 160, 208, 68, r, false);
}
inline void progress(Adafruit_GFX& g, int filled, int total, int y) {                // segmented bar
  int w = 28, gap = 6, x0 = 120 - (total * w + (total - 1) * gap) / 2;
  for (int i = 0; i < total; i++) g.fillRoundRect(x0 + i * (w + gap), y, w, 8, 4, i < filled ? ORANGE : TRACK);
}
inline void tree(Adafruit_GFX& g, int x, int y) { g.fillRect(x - 2, y + 8, 5, 10, WOOD); g.fillCircle(x, y, 10, GREEN); }
inline void house(Adafruit_GFX& g, int x, int y) { g.fillTriangle(x - 12, y + 10, x, y - 6, x + 12, y + 10, ORANGE); g.fillRect(x - 9, y + 10, 18, 7, HOUSE_WALL); }
inline void pet(Adafruit_GFX& g, int cx, int cy, uint8_t mood, int la, int ra) { maeum::drawCloud(g, cx, cy, 100, 2, mood, true, la, ra); }
inline void sparks(Adafruit_GFX& g, int x, int y) {
  thickLine(g, x - 8, y + 6, x - 14, y - 2, ORANGE); thickLine(g, x, y, x, y - 10, ORANGE); thickLine(g, x + 8, y + 6, x + 14, y - 2, ORANGE);
}

// ---------- backdrops ----------
inline void roomBase(Adafruit_GFX& g) {
  g.fillScreen(WALL);
  g.fillRect(0, 156, 240, 84, FLOOR); g.fillRect(0, 152, 240, 6, LINE);
  g.fillRect(24, 34, 60, 50, CREAM); g.fillRect(28, 38, 52, 42, SKY);                   // window
  g.fillRect(52, 38, 3, 42, CREAM); g.fillRect(28, 57, 52, 3, CREAM); g.fillCircle(40, 48, 7, CELESTIAL);
}
inline void bedroom(Adafruit_GFX& g, bool made) {
  roomBase(g);
  g.fillRoundRect(120, 124, 104, 48, 4, WOOD); g.fillRect(216, 92, 12, 80, WOOD_D);      // bed
  g.fillRoundRect(128, 110, 88, 28, 6, CREAM); g.fillRoundRect(184, 98, 28, 18, 6, PILLOW);
  if (made) {
    g.fillRoundRect(136, 118, 66, 26, 4, ORANGE);
  } else {                                                                              // messy blanket
    g.fillTriangle(128, 124, 192, 116, 196, 144, ORANGE); g.fillTriangle(128, 124, 196, 144, 128, 144, ORANGE);
    g.fillTriangle(192, 116, 176, 132, 196, 144, ORANGE_D);
  }
}
inline void bathroom(Adafruit_GFX& g, bool sink) {
  g.fillScreen(WALL);
  for (int y = 40; y < 156; y += 24) g.fillRect(0, y, 240, 2, TILE);                    // wall tiles
  for (int x = 0; x < 240; x += 24) g.fillRect(x, 26, 2, 130, TILE);
  g.fillRect(0, 156, 240, 84, FLOOR); g.fillRect(0, 152, 240, 6, LINE);
  if (sink) {
    g.fillRoundRect(156, 40, 62, 56, 6, CREAM); g.fillRoundRect(160, 44, 54, 48, 4, WATER_L);   // mirror
    thickLine(g, 168, 84, 186, 52, CREAM);
    g.fillRect(150, 118, 76, 38, WOOD); g.fillRoundRect(146, 106, 84, 14, 5, CREAM);              // cabinet and basin
    g.fillRect(184, 94, 6, 14, METAL); g.fillRect(178, 92, 18, 5, METAL);                         // faucet
  }
}
inline void kitchen(Adafruit_GFX& g) {
  g.fillScreen(WALL);
  g.fillRect(0, 156, 240, 84, FLOOR); g.fillRect(0, 152, 240, 6, LINE);
  g.fillRect(110, 36, 118, 36, WOOD_D); g.fillRect(168, 36, 2, 36, OUTLINE);                       // upper cabinet
  g.fillRect(150, 126, 86, 30, WOOD); g.fillRect(150, 120, 86, 8, METAL);                        // counter + stove top
}
inline void outdoor(Adafruit_GFX& g) {
  g.fillScreen(WALL);
  g.fillCircle(204, 56, 16, ORANGE);                                                           // sun
  ellipse(g, 70, 172, 100, 24, HILL);                                                          // hills
  g.fillRect(0, 170, 240, 70, FLOOR); g.fillRect(0, 182, 240, 22, PATH);                       // ground + path
  g.fillRect(24, 124, 8, 46, WOOD_D); g.fillCircle(28, 112, 22, GREEN);                        // tree
}

// ---------- screens ----------
inline void menuScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG);
  for (int i = 0; i < 5; i++) { if (i % 2 == 0) tree(g, 20 + i * 50, 14); else house(g, 20 + i * 50, 14); }
  g.fillRect(0, 34, 240, 4, WATER_L);
  g.fillRoundRect(18, 50, 204, 136, 12, ORANGE); g.fillRoundRect(21, 53, 198, 130, 10, CREAM);
  g.fillTriangle(34, 68, 48, 78, 34, 88, ORANGE);
  text(g, 66, 62, "미션", INK, 2); text(g, 66, 100, "체크인", INK, 2); text(g, 66, 138, "상점", INK, 2);
  g.fillRect(0, 198, 240, 4, WATER_L);
  for (int i = 0; i < 5; i++) { if (i % 2 == 1) tree(g, 20 + i * 50, 222); else house(g, 20 + i * 50, 222); }
}

inline void roomScene(Adafruit_GFX& g) {
  bedroom(g, true);
  ellipse(g, 84, 190, 46, 9, RUG);
  pet(g, 84, 148, maeum::MOOD_HAPPY, maeum::ARM_DOWN, maeum::ARM_DOWN);
  dashedRect(g, 96, 40, 28, 44, SLOT); g.fillRect(108, 58, 4, 10, SLOT); g.fillRect(105, 61, 10, 4, SLOT);
  dashedRect(g, 12, 128, 32, 44, SLOT); g.fillRect(26, 146, 4, 10, SLOT); g.fillRect(23, 149, 10, 4, SLOT);
  headerCoins(g, "내 방", "120");
  hints(g, "이전", "놓기", "다음");
}

// ---------- morning missions ----------
inline void makeBedScene(Adafruit_GFX& g) {
  bedroom(g, false);
  pet(g, 62, 116, maeum::MOOD_NORMAL, maeum::ARM_DOWN, 100);
  sparks(g, 128, 100);
  header(g, "침대 정리", "1 / 2");
  pill(g, 70, 206, 100, "정리 완료", true);
}
inline void drinkWaterScene(Adafruit_GFX& g) {
  roomBase(g);
  g.fillRect(128, 128, 90, 8, WOOD); g.fillRect(134, 136, 6, 20, WOOD_D); g.fillRect(206, 136, 6, 20, WOOD_D);   // table
  g.fillRoundRect(150, 96, 22, 32, 4, CREAM); g.fillRect(153, 108, 16, 17, DROP);                           // glass
  thickLine(g, 166, 88, 169, 106, ORANGE);                                                                    // straw
  pet(g, 92, 118, maeum::MOOD_NORMAL, maeum::ARM_DOWN, 100);
  sparks(g, 161, 80);
  header(g, "물 마시기", "1 / 2");
  pill(g, 70, 206, 100, "완료", true);
}
inline void washScene(Adafruit_GFX& g) {
  bathroom(g, true);
  pet(g, 66, 118, maeum::MOOD_HAPPY, maeum::ARM_DOWN, 130);
  int hx, hy; maeum::armHand(66, 118, 100, 2, true, 130, hx, hy);
  thickLine(g, hx, hy, 86, 126, METAL); g.fillRoundRect(78, 120, 14, 9, 3, ORANGE);                           // toothbrush
  g.fillCircle(96, 96, 6, DROP); g.fillCircle(108, 84, 4, DROP); g.fillCircle(90, 80, 3, DROP);      // bubbles
  header(g, "세수·양치", "1 / 2");
  pill(g, 70, 206, 100, "완료", true);
}
inline void stretchScene(Adafruit_GFX& g) {
  roomBase(g);
  g.fillRoundRect(40, 164, 130, 10, 4, ORANGE_D);                                                              // mat
  pet(g, 104, 124, maeum::MOOD_HAPPY, maeum::ARM_UP, maeum::ARM_UP);
  int lx, ly, rx, ry; maeum::armHand(104, 124, 100, 2, false, maeum::ARM_UP, lx, ly); maeum::armHand(104, 124, 100, 2, true, maeum::ARM_UP, rx, ry);
  sparks(g, lx - 8, ly - 12); sparks(g, rx + 8, ry - 12);
  header(g, "스트레칭", "1 / 2");
  textC(g, 104, 180, "팔을 쭉 뻗어요", TEXT);
  progress(g, 1, 3, 214);
}
inline void airOutScene(Adafruit_GFX& g) {
  roomBase(g);
  g.fillTriangle(24, 34, 42, 40, 42, 76, CREAM); g.fillTriangle(24, 34, 42, 76, 24, 84, CREAM);                // open sash
  for (int i = 0; i < 3; i++) thickLine(g, 94, 50 + i * 14, 140 - i * 10, 50 + i * 14, METAL);                  // wind
  g.fillTriangle(150, 70, 160, 74, 152, 82, GREEN);
  pet(g, 112, 118, maeum::MOOD_HAPPY, maeum::ARM_UP, maeum::ARM_DOWN);
  header(g, "환기하기", "1 / 2");
  pill(g, 70, 206, 100, "완료", true);
}

// ---------- afternoon missions ----------
inline void walkScene(Adafruit_GFX& g) {
  outdoor(g);
  g.fillCircle(24, 186, 6, METAL); g.fillCircle(10, 190, 4, METAL);                                           // dust
  pet(g, 112, 138, maeum::MOOD_HAPPY, 50, 120);
  header(g, "산책하기", "1 / 1");
  textC(g, 120, 202, "걸음 320 / 1000", TEXT);
  g.fillRoundRect(40, 226, 160, 8, 4, TRACK); g.fillRoundRect(40, 226, 52, 8, 4, ORANGE);
}
inline void showerScene(Adafruit_GFX& g) {
  bathroom(g, false);
  g.fillRect(148, 30, 6, 66, METAL); g.fillRect(148, 30, 50, 6, METAL); g.fillRoundRect(176, 36, 32, 10, 5, METAL);   // pipe + head
  for (int x = 180; x <= 204; x += 8) for (int y = 52; y < 112; y += 16) g.fillRect(x, y, 2, 9, DROP);
  pet(g, 190, 124, maeum::MOOD_HAPPY, maeum::ARM_UP, maeum::ARM_UP);
  const int fx[] = {176, 190, 204}, fy[] = {98, 92, 99}, fr[] = {8, 10, 7};
  for (int i = 0; i < 3; i++) g.fillCircle(fx[i], fy[i], fr[i] + 2, OUTLINE);
  for (int i = 0; i < 3; i++) g.fillCircle(fx[i], fy[i], fr[i], PILLOW);
  header(g, "샤워하기", "1 / 1");
  pill(g, 12, 206, 100, "완료", true);
}
inline void cleanScene(Adafruit_GFX& g) {
  roomBase(g);
  pet(g, 70, 120, maeum::MOOD_NORMAL, maeum::ARM_DOWN, 100);
  int hx, hy; maeum::armHand(70, 120, 100, 2, true, 100, hx, hy);
  thickLine(g, hx - 6, hy - 44, hx + 12, hy + 26, WOOD);                                                      // broom handle
  g.fillTriangle(hx + 2, hy + 22, hx + 30, hy + 30, hx + 8, hy + 40, ORANGE_D);                               // bristles
  g.fillCircle(hx + 40, hy + 36, 4, METAL); g.fillCircle(hx + 52, hy + 40, 3, METAL); g.fillCircle(hx + 28, hy + 46, 3, METAL);
  header(g, "방 청소", "1 / 1");
  pill(g, 70, 206, 100, "완료", true);
}
inline void groceriesScene(Adafruit_GFX& g) {
  outdoor(g);
  g.fillRect(140, 86, 92, 84, PILLOW); g.fillRect(140, 78, 92, 12, ORANGE);                                    // store + awning
  for (int x = 150; x < 232; x += 24) g.fillRect(x, 78, 12, 12, CREAM);
  g.fillRect(176, 124, 26, 46, WOOD_D); g.fillRect(148, 104, 20, 18, SKY);
  pet(g, 70, 134, maeum::MOOD_HAPPY, maeum::ARM_DOWN, 80);
  int hx, hy; maeum::armHand(70, 134, 100, 2, true, 80, hx, hy);
  g.fillRect(hx - 6, hy + 2, 22, 24, WOOD); g.fillTriangle(hx - 2, hy + 2, hx + 4, hy - 8, hx + 8, hy + 2, GREEN); g.fillTriangle(hx + 6, hy + 2, hx + 12, hy - 6, hx + 16, hy + 2, GREEN);
  header(g, "장보기", "1 / 1");
  textC(g, 120, 202, "걸음 120 / 800", TEXT);
  g.fillRoundRect(40, 226, 160, 8, 4, TRACK); g.fillRoundRect(40, 226, 24, 8, 4, ORANGE);
}
inline void cookScene(Adafruit_GFX& g) {
  kitchen(g);
  g.fillRoundRect(164, 98, 54, 24, 4, METAL); g.fillRect(160, 96, 62, 5, METAL);                               // pot + rim
  g.fillCircle(176, 84, 6, STEAM); g.fillCircle(192, 74, 7, STEAM); g.fillCircle(208, 84, 6, STEAM);        // steam
  pet(g, 84, 122, maeum::MOOD_HAPPY, maeum::ARM_DOWN, 130);
  int hx, hy; maeum::armHand(84, 122, 100, 2, true, 130, hx, hy);
  thickLine(g, hx, hy, hx + 22, hy - 30, WOOD); g.fillCircle(hx + 24, hy - 33, 7, WOOD);                       // ladle
  header(g, "요리하기", "1 / 1");
  pill(g, 70, 206, 100, "완료", true);
}
inline void studyScene(Adafruit_GFX& g) {
  roomBase(g);
  g.fillRect(122, 134, 108, 8, WOOD); g.fillRect(128, 142, 8, 14, WOOD_D); g.fillRect(216, 142, 8, 14, WOOD_D);   // desk
  g.fillRoundRect(142, 118, 34, 16, 2, CREAM); g.fillRoundRect(178, 118, 34, 16, 2, CREAM);                       // open book
  for (int i = 0; i < 3; i++) { g.fillRect(147, 122 + i * 4, 24, 1, LINE); g.fillRect(183, 122 + i * 4, 24, 1, LINE); }
  g.fillRect(212, 126, 18, 8, METAL); thickLine(g, 220, 126, 214, 100, METAL); g.fillTriangle(196, 92, 232, 92, 214, 72, ORANGE);   // lamp
  pet(g, 80, 122, maeum::MOOD_NORMAL, maeum::ARM_DOWN, 95);
  int hx, hy; maeum::armHand(80, 122, 100, 2, true, 95, hx, hy);
  thickLine(g, hx, hy, 148, 122, ORANGE_D);                                                                       // pencil
  header(g, "공부하기", "1 / 1");
  pill(g, 12, 206, 100, "완료", true);
}

// ---------- special missions ----------
inline void fishingScene(Adafruit_GFX& g) {
  g.fillScreen(WALL);
  g.fillRect(0, 140, 240, 100, WATER);
  const int rx[] = {104, 160, 60, 192}, ry[] = {168, 184, 204, 156}, rw[] = {20, 24, 24, 20};
  for (int i = 0; i < 4; i++) g.fillRoundRect(rx[i], ry[i], rw[i], 3, 1, WATER_L);
  g.fillRect(0, 124, 96, 16, WOOD); g.fillRect(76, 140, 8, 32, WOOD_D);
  pet(g, 46, 84, maeum::MOOD_NORMAL, maeum::ARM_DOWN, 125);
  int hx, hy; maeum::armHand(46, 84, 100, 2, true, 125, hx, hy);
  thickLine(g, hx, hy, 140, 66, ROD); g.drawFastVLine(140, 66, 100, METAL);
  g.fillCircle(140, 168, 6, ORANGE);
  ellipse(g, 156, 196, 18, 8, SHADOW); g.fillTriangle(172, 196, 188, 188, 188, 204, SHADOW);
  g.fillRect(138, 36, 5, 16, ORANGE); g.fillRect(138, 56, 5, 5, ORANGE);
  header(g, "낚시하기", "1 / 3");
  textC(g, 120, 208, "기기를 흔들어요!", TEXT);
}
inline void waterPlantScene(Adafruit_GFX& g) {
  roomBase(g);
  g.fillRoundRect(158, 136, 40, 22, 4, ORANGE_D); g.fillRect(158, 133, 40, 6, WOOD_D);                          // pot + soil
  g.fillRect(176, 108, 4, 28, GREEN); g.fillTriangle(178, 124, 156, 112, 164, 130, GREEN); g.fillTriangle(178, 116, 200, 104, 192, 122, GREEN);
  pet(g, 78, 124, maeum::MOOD_HAPPY, maeum::ARM_DOWN, 100);
  int hx, hy; maeum::armHand(78, 124, 100, 2, true, 100, hx, hy);
  g.fillRoundRect(hx - 4, hy - 16, 26, 24, 4, METAL); thickLine(g, hx + 22, hy - 8, hx + 36, hy - 20, METAL);   // watering can
  for (int i = 0; i < 3; i++) g.fillRect(hx + 38 + i, hy - 14 + i * 12, 3, 7, DROP);                           // drops
  header(g, "식물 물주기", "1 / 3");
  pill(g, 70, 206, 100, "물 주기", true);
}

}  // namespace maeum_scene
