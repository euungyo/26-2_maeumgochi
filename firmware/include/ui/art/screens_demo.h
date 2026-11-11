// screens_demo.h - the 10 screens from the 3-button table (Korean text). Needs scenes_demo.h, cloud_sprite.h, ko_text.h, ko_font.h.
// 1 mainScreen  2 lifeMenuScreen  3 charMenuScreen  4 missionListScreen  5 missionDetailScreen
// 6 checkinScreen  7 confirmScreen  8 shopListScreen  9 itemDetailScreen  10 bagScreen
// 11 growthScreen  12 growthUpScreen  13-15 mealScreen(0 breakfast, 1 lunch, 2 dinner)  16 sleepScreen
// 17 recordScreen  18 lowBatteryScreen  19 savedScreen (main screen + 'saved' toast; savedToast() can be drawn over any screen)
#pragma once
#include "scenes_demo.h"

namespace maeum_screen {
using namespace maeum_scene;

// ---------- small icons ----------
inline void iconHat(Adafruit_GFX& g, int x, int y) {                      // 32 x 32 box
  for (int pass = 0; pass < 2; pass++) {
    int o = pass == 0 ? 2 : 0; uint16_t c = pass == 0 ? OUTLINE : ORANGE;
    g.fillRoundRect(x + 6 - o, y + 7 - o, 20 + 2 * o, 17 + 2 * o, 9, c);
    g.fillRoundRect(x + 3 - o, y + 21 - o, 26 + 2 * o, 7 + 2 * o, 3, c);
    g.fillCircle(x + 16, y + 6, 4 + o, pass == 0 ? OUTLINE : CREAM);
  }
}
inline void iconBow(Adafruit_GFX& g, int x, int y) {
  for (int pass = 0; pass < 2; pass++) {
    int o = pass == 0 ? 2 : 0; uint16_t c = pass == 0 ? OUTLINE : maeum::C_BLUSH;
    g.fillTriangle(x + 16, y + 16, x + 2 - o, y + 6 - o, x + 2 - o, y + 26 + o, c);
    g.fillTriangle(x + 16, y + 16, x + 30 + o, y + 6 - o, x + 30 + o, y + 26 + o, c);
    g.fillCircle(x + 16, y + 16, 5 + o, pass == 0 ? OUTLINE : ORANGE);
  }
}
inline void iconLamp(Adafruit_GFX& g, int x, int y) {
  g.fillTriangle(x + 4, y + 16, x + 28, y + 16, x + 16, y + 2, ORANGE);
  g.fillRect(x + 14, y + 16, 4, 10, METAL); g.fillRoundRect(x + 8, y + 25, 16, 5, 2, METAL);
}
inline void drop(Adafruit_GFX& g, int cx, int cy) { g.fillTriangle(cx, cy - 8, cx - 5, cy, cx + 5, cy, DROP); g.fillCircle(cx, cy + 1, 5, DROP); }
inline void checkMark(Adafruit_GFX& g, int cx, int cy, bool done) {
  if (done) { g.fillCircle(cx, cy, 10, ORANGE); thickLine(g, cx - 5, cy, cx - 1, cy + 4, CREAM); thickLine(g, cx - 1, cy + 4, cx + 6, cy - 4, CREAM); }
  else { g.fillCircle(cx, cy, 10, PILL_OFF_BORDER); g.fillCircle(cx, cy, 8, PILL_OFF_BG); }
}
inline void arcDots(Adafruit_GFX& g, int cx, int cy, int halfW, int depth, int dir, int th, uint16_t c) {
  for (int i = 0; i <= 14; i++) {
    float u = -1.0f + 2.0f * i / 14.0f;
    g.fillCircle(cx + (int)lroundf(u * halfW), cy + (int)lroundf(dir * depth * (1.0f - u * u)), th, c);
  }
}
inline void face(Adafruit_GFX& g, int cx, int cy, int r, int level, bool selected) {      // level 1..5
  if (selected) g.fillCircle(cx, cy, r + 6, ORANGE);
  g.fillCircle(cx, cy, r + 2, OUTLINE); g.fillCircle(cx, cy, r, CREAM);
  int er = r >= 18 ? 2 : 1;
  g.fillCircle(cx - r / 3, cy - r / 6, er, INK); g.fillCircle(cx + r / 3, cy - r / 6, er, INK);
  int my = cy + r / 2, hw = r / 2, th = r >= 18 ? 1 : 1;
  if (level == 1) arcDots(g, cx, my + 2, hw, r / 3, -1, th, INK);
  else if (level == 2) arcDots(g, cx, my + 1, hw, r / 5, -1, th, INK);
  else if (level == 3) g.fillRect(cx - hw, my, 2 * hw, 2, INK);
  else if (level == 4) arcDots(g, cx, my - 2, hw, r / 4, +1, th, INK);
  else arcDots(g, cx, my - 3, hw + 1, r / 2, +1, th, INK);
}
inline void menuList(Adafruit_GFX& g, const char* title, const char* a, const char* b, const char* c, int sel) {
  g.fillScreen(MENU_BG); header(g, title, nullptr);
  g.fillRoundRect(16, 38, 208, 160, 14, ORANGE); g.fillRoundRect(19, 41, 202, 154, 12, CREAM);
  const char* it[3] = {a, b, c};
  for (int i = 0; i < 3; i++) {
    int y = 52 + i * 46;
    if (i == sel) { g.fillRoundRect(26, y - 6, 188, 44, 10, SEL_BG); g.fillTriangle(36, y + 6, 36, y + 26, 50, y + 16, ORANGE); }
    text(g, 70, y + 4, it[i], INK, 2);
  }
}

// ---------- 1. main ----------
inline void mainScreen(Adafruit_GFX& g) {
  roomBase(g);
  ellipse(g, 120, 168, 52, 9, RUG);
  maeum::drawCloud(g, 120, 124, 120, 2, maeum::MOOD_NORMAL, true);
  g.fillRect(0, 0, 240, 26, BAR); text(g, 8, 5, "09:24", BAR_TEXT); textR(g, 232, 5, "120", BAR_TEXT); coin(g, 232 - maeum::ko::width("120") - 14, 13);
  g.fillRoundRect(52, 178, 136, 26, 13, OUTLINE); g.fillRoundRect(54, 180, 132, 22, 11, CREAM);
  drop(g, 72, 189); text(g, 88, 182, "물 마시기", INK);
  hints(g, "생활", "미션", "캐릭터");
}
// ---------- 2, 3. menus ----------
inline void lifeMenuScreen(Adafruit_GFX& g) { menuList(g, "생활", "미션", "체크인", "기록", 0); hints(g, "이전", "선택", "다음"); }
inline void charMenuScreen(Adafruit_GFX& g) { menuList(g, "캐릭터", "성장", "상점", "가방", 1); hints(g, "이전", "선택", "다음"); }

// ---------- 4. mission list ----------
inline void missionListScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG); header(g, "오늘의 미션", "1 / 3");
  const char* name[3] = {"침대 정리", "스트레칭", "산책하기"}; const char* tag[3] = {"아침", "아침", "오후"};
  for (int i = 0; i < 3; i++) {
    int y = 38 + i * 56;
    g.fillRoundRect(14, y, 212, 50, 12, i == 1 ? ORANGE : PILL_OFF_BORDER);
    g.fillRoundRect(17, y + 3, 206, 44, 10, i == 1 ? SEL_BG : CREAM);
    checkMark(g, 40, y + 25, i == 0);
    text(g, 64, y + 17, name[i], INK);
    textR(g, 210, y + 17, tag[i], SUB_TEXT);
  }
  hints(g, "이전", "상세", "다음");
}
// ---------- 5. mission detail ----------
inline void missionDetailScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG); header(g, "스트레칭", "자동 확인");
  maeum::drawCloud(g, 120, 84, 100, 2, maeum::MOOD_HAPPY, true, maeum::ARM_UP, maeum::ARM_UP);
  textC(g, 120, 130, "팔을 쭉 뻗어 3번 해요", INK);
  progress(g, 0, 3, 158);
  textC(g, 120, 176, "경험치 +10   코인 +5", SUB_TEXT);
  hints(g, "이전", "시작", "다음");
}
// ---------- 6. check-in ----------
inline void checkinScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG); header(g, "체크인", "1 / 2");
  textC(g, 120, 44, "오늘 기분은 어때요?", INK);
  const int xs[5] = {28, 74, 120, 166, 212};
  for (int i = 0; i < 5; i++) { bool sel = (i == 3); face(g, xs[i], 108, sel ? 20 : 14, i + 1, sel); }
  textC(g, 120, 146, "4", INK, 2);
  textC(g, 120, 184, "좋아요", SUB_TEXT);
  hints(g, "-", "확정", "+");
}
// ---------- 7. confirm ----------
inline void confirmScreen(Adafruit_GFX& g) {
  roomBase(g);
  maeum::drawCloud(g, 120, 62, 78, 2, maeum::MOOD_HAPPY, true, maeum::ARM_UP, maeum::ARM_UP);
  g.fillRoundRect(26, 106, 188, 92, 14, OUTLINE); g.fillRoundRect(29, 109, 182, 86, 12, CREAM);
  textC(g, 120, 120, "미션을 완료했나요?", INK);
  g.fillRoundRect(42, 154, 70, 30, 8, PILL_OFF_BORDER); g.fillRoundRect(44, 156, 66, 26, 7, PILL_OFF_BG); textC(g, 77, 161, "취소", PILL_OFF_TEXT);
  g.fillRoundRect(128, 154, 70, 30, 8, ORANGE); textC(g, 163, 161, "완료", INK);
  g.fillRect(0, 0, 240, 26, BAR); text(g, 8, 5, "스트레칭", BAR_TEXT);
  hints(g, "취소", "확정", "완료");
}
// ---------- 8. shop list ----------
inline void shopListScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG); headerCoins(g, "상점", "120");
  const char* name[3] = {"모자", "리본", "스탠드"}; const char* price[3] = {"30", "20", "40"};
  for (int i = 0; i < 3; i++) {
    int y = 36 + i * 56;
    g.fillRoundRect(14, y, 212, 50, 12, i == 0 ? ORANGE : PILL_OFF_BORDER);
    g.fillRoundRect(17, y + 3, 206, 44, 10, i == 0 ? SEL_BG : CREAM);
    if (i == 0) iconHat(g, 26, y + 9); else if (i == 1) iconBow(g, 26, y + 9); else iconLamp(g, 26, y + 9);
    text(g, 72, y + 17, name[i], INK);
    textR(g, 208, y + 17, price[i], INK); coin(g, 208 - maeum::ko::width(price[i]) - 14, y + 25);
  }
  hints(g, "이전", "상세", "다음");
}
// ---------- 9. item detail ----------
inline void itemDetailScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG); header(g, "모자", "1 / 2");
  maeum::drawCloud(g, 120, 100, 110, 2, maeum::MOOD_HAPPY, true);
  maeum::drawHat(g, 120, 100, 110, 2);
  g.fillTriangle(58, 152, 70, 145, 70, 159, SUB_TEXT); g.fillTriangle(182, 152, 170, 145, 170, 159, SUB_TEXT);
  textC(g, 120, 144, "색상  주황", INK);
  coin(g, 108, 176); textC(g, 138, 168, "30", INK);
  hints(g, "이전", "구매", "다음");
}
// ---------- 10. bag ----------
inline void bagScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG); header(g, "가방", "1 / 3");
  maeum::drawCloud(g, 72, 96, 90, 2, maeum::MOOD_HAPPY, true);
  maeum::drawHat(g, 72, 96, 90, 2);
  text(g, 130, 52, "모자", INK, 2);
  g.fillCircle(138, 108, 9, ORANGE); thickLine(g, 133, 108, 137, 112, CREAM); thickLine(g, 137, 112, 144, 104, CREAM);
  text(g, 154, 100, "착용 중", SUB_TEXT);
  for (int i = 0; i < 3; i++) {
    int x = 40 + i * 62;
    g.fillRoundRect(x, 146, 48, 48, 10, i == 0 ? ORANGE : PILL_OFF_BORDER); g.fillRoundRect(x + 3, 149, 42, 42, 8, i == 0 ? SEL_BG : CREAM);
    if (i == 0) iconHat(g, x + 8, 154); else if (i == 1) iconBow(g, x + 8, 154); else iconLamp(g, x + 8, 154);
  }
  hints(g, "이전", "해제", "다음");
}

// ---------- 11. growth (character menu > growth) ----------
inline void growthScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG); header(g, "성장", "2단계");
  const int xs[3] = {40, 120, 200}; const char* lb[3] = {"1단계", "2단계", "3단계"};
  g.fillRoundRect(84, 44, 72, 110, 12, ORANGE); g.fillRoundRect(87, 47, 66, 104, 10, SEL_BG);       // current stage
  for (int i = 0; i < 3; i++) {
    maeum::drawCloud(g, xs[i], 92, 74, (uint8_t)(i + 1), maeum::MOOD_NORMAL, true, maeum::ARM_NONE, maeum::ARM_NONE);
    textC(g, xs[i], 128, lb[i], i == 1 ? INK : SUB_TEXT);
  }
  textC(g, 120, 164, "다음 단계까지 미션 1개", INK);
  progress(g, 1, 2, 188);
  hints(g, "", "확인", "");
}
// ---------- 12. growth moment ----------
inline void growthUpScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG);
  const int cx[] = {26, 52, 196, 220, 40, 206, 100, 150, 18, 228, 70, 172}, cy[] = {40, 90, 36, 84, 128, 132, 24, 30, 70, 112, 60, 62};
  for (int i = 0; i < 12; i++) g.fillCircle(cx[i], cy[i], (i % 3) + 3, i % 3 == 0 ? ORANGE : (i % 3 == 1 ? maeum::C_BLUSH : GREEN));
  maeum::drawCloud(g, 120, 86, 120, 3, maeum::MOOD_HAPPY, true, maeum::ARM_UP, maeum::ARM_UP);
  textC(g, 120, 150, "성장했어요!", INK, 2);
  textC(g, 120, 186, "이제 3단계예요", SUB_TEXT);
  hints(g, "", "확인", "");
}
// ---------- 13-15. meal time ----------
inline void windowSky(Adafruit_GFX& g, uint16_t sky, uint16_t cel, int cy) {
  g.fillRect(28, 38, 52, 42, sky); g.fillRect(52, 38, 3, 42, CREAM); g.fillRect(28, 57, 52, 3, CREAM); g.fillCircle(40, cy, 7, cel);
}
inline void riceBowl(Adafruit_GFX& g, int x, int y) { g.fillCircle(x + 17, y + 6, 13, PILLOW); g.fillRoundRect(x, y + 4, 34, 15, 7, METAL); }
inline void soupBowl(Adafruit_GFX& g, int x, int y) {
  g.fillRoundRect(x, y + 4, 34, 15, 7, ORANGE_D); g.fillRect(x + 3, y + 4, 28, 4, WOOD_D);
  g.fillCircle(x + 10, y - 6, 4, STEAM); g.fillCircle(x + 22, y - 12, 5, STEAM);
}
inline void mealScreen(Adafruit_GFX& g, int meal) {                       // 0 breakfast, 1 lunch, 2 dinner
  roomBase(g);
  if (meal == 0) windowSky(g, rgb(0xDD, 0xEE, 0xF8), ORANGE, 68);
  else if (meal == 1) windowSky(g, SKY, ORANGE, 46);
  else windowSky(g, rgb(0xF2, 0xC7, 0xA5), ORANGE_D, 70);
  g.fillRoundRect(106, 142, 118, 10, 4, WOOD); g.fillRect(118, 152, 8, 8, WOOD_D); g.fillRect(206, 152, 8, 8, WOOD_D);   // table
  if (meal == 0) { g.fillRoundRect(124, 130, 36, 14, 7, CREAM); g.fillRoundRect(130, 126, 24, 10, 3, WOOD); g.fillCircle(188, 136, 9, CREAM); g.fillCircle(188, 136, 4, ORANGE); }
  else { riceBowl(g, 122, 124); soupBowl(g, 168, 124); if (meal == 2) { g.fillRoundRect(204, 134, 18, 8, 4, CREAM); g.fillCircle(212, 132, 4, GREEN); } }
  maeum::drawCloud(g, 62, 122, 100, 2, maeum::MOOD_HAPPY, true, maeum::ARM_DOWN, 100);
  int hx, hy; maeum::armHand(62, 122, 100, 2, true, 100, hx, hy);
  thickLine(g, hx, hy, hx + 14, hy - 8, METAL); g.fillCircle(hx + 16, hy - 9, 4, METAL);                                    // spoon
  const char* tm[3] = {"07:30", "13:00", "19:00"}; const char* nm[3] = {"아침 시간", "점심 시간", "저녁 시간"};
  header(g, tm[meal], nm[meal]);
  textC(g, 120, 178, "밥 먹을 시간이에요", TEXT);
  pill(g, 12, 208, 68, "나중에", false); pill(g, 90, 208, 100, "먹었어요", true);
}
inline void mealBreakfast(Adafruit_GFX& g) { mealScreen(g, 0); }
inline void mealLunch(Adafruit_GFX& g) { mealScreen(g, 1); }
inline void mealDinner(Adafruit_GFX& g) { mealScreen(g, 2); }
// ---------- 16. sleep (always dark and quiet) ----------
inline void sleepScreen(Adafruit_GFX& g) {
  const uint16_t NIGHT = rgb(0x14, 0x1E, 0x3A), DEEP = rgb(0x0C, 0x14, 0x28), BLANKET = rgb(0x3A, 0x4C, 0x7C), STRIPE = rgb(0x52, 0x66, 0x9A);
  const uint16_t STAR = rgb(0xF4, 0xE9, 0xB8), DIM = rgb(0x8F, 0xA3, 0xC8), PIL = rgb(0xC9, 0xD4, 0xEC);
  g.fillScreen(NIGHT);
  g.fillRect(154, 30, 62, 54, rgb(0x5C, 0x6E, 0xA0)); g.fillRect(158, 34, 54, 46, DEEP);                                       // window
  g.fillCircle(174, 56, 9, STAR); g.fillCircle(179, 52, 8, DEEP);                                                              // moon
  g.fillRect(196, 46, 3, 3, STAR); g.fillRect(190, 68, 3, 3, STAR); g.fillRect(204, 62, 3, 3, STAR);
  text(g, 20, 34, "23:40", DIM, 2);
  g.fillRoundRect(20, 150, 200, 52, 12, rgb(0x5A, 0x46, 0x38));                                                                 // bed
  g.fillRoundRect(40, 128, 84, 30, 14, PIL);                                                                                   // pillow
  maeum::drawCloud(g, 90, 112, 100, 2, maeum::MOOD_SLEEPY, false, maeum::ARM_NONE, maeum::ARM_NONE);
  g.fillRoundRect(30, 140, 180, 62, 16, BLANKET);                                                                              // blanket
  for (int x = 44; x < 200; x += 22) g.fillRect(x, 146, 8, 52, STRIPE);
  text(g, 132, 94, "Z z z", DIM, 2);
  textC(g, 120, 212, "잘 자요", DIM);
}

// ---------- 17. record (life menu > record) ----------
inline void recordScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG); header(g, "기록", "이번 주");
  const int cnt[7] = {2, 3, 1, 3, 2, 0, 3}, mood[7] = {3, 4, 2, 4, 3, 0, 5};
  const char* day[7] = {"월", "화", "수", "목", "금", "토", "일"};
  text(g, 14, 36, "미션 14 / 21", INK); textR(g, 226, 36, "평균 기분 3.5", SUB_TEXT);
  for (int i = 0; i < 7; i++) {
    int cx = 30 + i * 30, bx = cx - 9;
    if (mood[i]) face(g, cx, 84, 9, mood[i], false); else g.fillCircle(cx, 84, 4, PILL_OFF_BORDER);
    g.fillRoundRect(bx, 104, 18, 66, 6, TRACK);
    if (cnt[i]) g.fillRoundRect(bx, 170 - cnt[i] * 22, 18, cnt[i] * 22, 6, ORANGE);
    textC(g, cx, 178, day[i], i == 6 ? INK : SUB_TEXT);
  }
  hints(g, "이전 주", "", "다음 주");
}
// ---------- 18. low battery ----------
inline void lowBatteryScreen(Adafruit_GFX& g) {
  g.fillScreen(MENU_BG); header(g, "배터리", nullptr);
  g.fillRoundRect(78, 50, 84, 44, 10, OUTLINE); g.fillRoundRect(81, 53, 78, 38, 8, CREAM);       // battery body
  g.fillRoundRect(162, 63, 8, 18, 3, OUTLINE);                                                    // battery tip
  g.fillRoundRect(86, 58, 16, 28, 4, ORANGE);                                                     // one cell left
  maeum::drawCloud(g, 120, 138, 70, 2, maeum::MOOD_SLEEPY, true);
  textC(g, 120, 170, "배터리가 부족해요", INK);
  textC(g, 120, 190, "기록은 안전하게 저장돼요", SUB_TEXT);
  hints(g, "", "확인", "");
}
// ---------- 19. saved toast ----------
inline void savedToast(Adafruit_GFX& g) {
  g.fillRoundRect(46, 34, 148, 30, 15, BAR);
  g.fillCircle(68, 49, 10, ORANGE); thickLine(g, 63, 49, 67, 53, CREAM); thickLine(g, 67, 53, 74, 44, CREAM);
  text(g, 86, 41, "저장됐어요", BAR_TEXT);
}
inline void savedScreen(Adafruit_GFX& g) { mainScreen(g); savedToast(g); }

}  // namespace maeum_screen
