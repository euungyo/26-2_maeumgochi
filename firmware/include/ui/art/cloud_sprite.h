// cloud_sprite.h - Maeumgochi cloud pet drawn with Adafruit_GFX primitives only (no bitmaps).
// Usage: maeum::drawCloud(tft, 120, 100, 150, 2, maeum::MOOD_NORMAL);
//   tft      : any Adafruit_GFX display (e.g. Adafruit_ST7789)
//   cx, cy   : center of the cloud in pixels
//   widthPx  : width of the STAGE 3 cloud in pixels (stage 1 and 2 are drawn smaller)
//   stage    : 1..3 (growth stage)
//   mood     : MOOD_SLEEPY / MOOD_NORMAL / MOOD_HAPPY
//   feet     : optional, true (default) draws two small feet under the cloud
//   leftArm / rightArm : optional arm angle in degrees (0 = hanging down, 90 = out to the side, 135 = raised), ARM_NONE hides the arm
//   maeum::armHand(...) returns the pixel position of a hand so props (rod, brush, bag) can be attached
#pragma once
#include <Adafruit_GFX.h>
#include <math.h>

namespace maeum {

// Color theme: 1 = Soft day (light, default), 0 = deep navy (night). Choose with  -DMAEUM_THEME=0  or  #define MAEUM_THEME 0  before including.
#ifndef MAEUM_THEME
#define MAEUM_THEME 1
#endif

constexpr uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b) { return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)); }

// RGB565 colors (from the design palette)
#if MAEUM_THEME == 1
const uint16_t C_NAVY    = rgb565(0x26, 0x39, 0x5A);   // ink: eyes and mouth, text on light shapes
const uint16_t C_OUTLINE = rgb565(0x70, 0x88, 0xB3);   // softer blue outline for light backgrounds
#else
const uint16_t C_NAVY    = 0x1127;                     // #12263B background / eyes
const uint16_t C_OUTLINE = 0x2A2C;                     // #2E4560 outline
#endif
const uint16_t C_CREAM   = 0xF79C;                     // #F4F0E6 body
const uint16_t C_ORANGE  = 0xF54D;                     // #F0A868 accent
const uint16_t C_BLUSH   = 0xFDD2;                     // light peach cheeks

enum Mood : uint8_t { MOOD_SLEEPY = 0, MOOD_NORMAL = 1, MOOD_HAPPY = 2 };

const int ARM_NONE = -1, ARM_DOWN = 35, ARM_OUT = 90, ARM_UP = 135;   // degrees, see above

namespace detail {

struct Blob { int8_t x, y, r; };   // circle in a 100 x 80 design box

static const Blob STAGE1[] = {{50,50,27},{25,50,17},{75,50,17},{39,35,15},{61,35,15},{47,20,9},{30,64,10},{42,66,10},{54,66,10},{66,64,10}};
static const Blob STAGE2[] = {{50,46,28},{22,46,17},{78,46,18},{35,27,17},{63,25,18},{26,62,11},{38,64,11},{50,64,11},{62,64,11},{74,62,11}};
static const Blob STAGE3[] = {{50,46,28},{22,50,18},{80,48,18},{38,31,20},{64,29,22},{28,64,11},{40,66,11},{52,66,11},{64,66,11},{76,64,11}};

struct Xf {                         // design units -> pixels
  int cx, cy; float s;
  int X(float x) const { return cx + (int)lroundf((x - 50.0f) * s); }
  int Y(float y) const { return cy + (int)lroundf((y - 40.0f) * s); }
  int R(float r, int mn = 1) const { int v = (int)lroundf(r * s); return v < mn ? mn : v; }
};

inline void dot(Adafruit_GFX& g, const Xf& t, float x, float y, float rad, uint16_t c) {
  g.fillCircle(t.X(x), t.Y(y), t.R(rad), c);
}

// thick curve helper: small arc "u" shaped (dir=+1, bowl) or arch (dir=-1)
inline void arc(Adafruit_GFX& g, const Xf& t, float x, float y, float halfW, float depth, int dir, float thick, uint16_t c) {
  for (int i = 0; i <= 14; i++) {
    float u = -1.0f + 2.0f * i / 14.0f;
    dot(g, t, x + u * halfW, y + dir * depth * (1.0f - u * u), thick, c);
  }
}

inline void curl(Adafruit_GFX& g, const Xf& t, float cx, float cy) {
  for (int i = 0; i <= 40; i++) {
    float a = 2.7f * 3.14159f * i / 40.0f;
    float r = 1.0f + a * 0.72f;
    dot(g, t, cx + r * cosf(a + 0.6f), cy + r * sinf(a + 0.6f), 1.35f, C_OUTLINE);
  }
}

inline void sparkle(Adafruit_GFX& g, const Xf& t, float x, float y, float big, float small) {
  g.fillTriangle(t.X(x), t.Y(y - big), t.X(x - small), t.Y(y), t.X(x + small), t.Y(y), C_ORANGE);
  g.fillTriangle(t.X(x), t.Y(y + big), t.X(x - small), t.Y(y), t.X(x + small), t.Y(y), C_ORANGE);
  g.fillTriangle(t.X(x - big), t.Y(y), t.X(x), t.Y(y - small), t.X(x), t.Y(y + small), C_ORANGE);
  g.fillTriangle(t.X(x + big), t.Y(y), t.X(x), t.Y(y - small), t.X(x), t.Y(y + small), C_ORANGE);
  for (int i = 0; i <= 5; i++) {   // two short dashes
    float u = i / 5.0f;
    dot(g, t, 92.0f + 5.0f * u, 21.0f + 3.0f * u, 1.3f, C_ORANGE);
    dot(g, t, 89.0f + 5.0f * u, 26.0f + 3.0f * u, 1.3f, C_ORANGE);
  }
}

inline void eyes(Adafruit_GFX& g, const Xf& t, uint8_t stage, uint8_t mood, float ex, float ey) {
  float er = (stage == 1) ? 7.0f : (stage == 2 ? 5.6f : 4.6f);
  for (int side = -1; side <= 1; side += 2) {
    float x = 50.0f + side * ex;
    if (mood == MOOD_NORMAL) {
      if (stage == 3) {                                   // tall oval eyes
        int w = t.R(4.8f), h = t.R(7.2f);
        g.fillRoundRect(t.X(x) - w, t.Y(ey) - h, 2 * w, 2 * h, w, C_NAVY);
      } else {
        dot(g, t, x, ey, er, C_NAVY);
        dot(g, t, x - er * 0.32f, ey - er * 0.32f, er * 0.32f, C_CREAM);   // highlight
      }
    } else if (mood == MOOD_SLEEPY) {
      arc(g, t, x, ey - 1.5f, 5.5f, 3.0f, +1, 1.3f, C_NAVY);              // closed, relaxed
    } else {
      arc(g, t, x, ey + 1.5f, 5.5f, 3.4f, -1, 1.3f, C_NAVY);              // happy arch
    }
  }
}


inline float stageScale(uint8_t stage) { return (stage == 1) ? 0.72f : (stage == 2 ? 0.86f : 1.0f); }
inline float shoulderDx(uint8_t stage) { return (stage == 1) ? 33.0f : (stage == 2 ? 36.0f : 38.0f); }
inline float shoulderY(uint8_t stage)  { return (stage == 1) ? 58.0f : 56.0f; }
#ifndef MAEUM_ARM_LEN
#define MAEUM_ARM_LEN 9.0f       // arm length (design units)
#endif
#ifndef MAEUM_ARM_R
#define MAEUM_ARM_R 3.2f         // arm thickness (radius)
#endif
#ifndef MAEUM_HAND_R
#define MAEUM_HAND_R 4.6f        // hand size (radius)
#endif
const float ARM_LEN = MAEUM_ARM_LEN;
inline float armLen(int deg) {                       // raised arms (over 90 degrees) get up to 80% longer so they clear the body
  float k = (deg > 90) ? (deg - 90) / 45.0f : 0.0f;
  if (k > 1.0f) k = 1.0f;
  return ARM_LEN * (1.0f + 0.8f * k);
}

inline void arms(Adafruit_GFX& g, const Xf& t, uint8_t stage, int leftDeg, int rightDeg, int pass) {
  // pass 0 = outline (drawn behind the body), pass 1 = cream (drawn in front, so the arm merges with the body)
  const float o = (pass == 0) ? 2.6f : 0.0f;
  const uint16_t col = (pass == 0) ? C_OUTLINE : C_CREAM;
  for (int side = -1; side <= 1; side += 2) {
    int deg = (side < 0) ? leftDeg : rightDeg;
    if (deg < 0) continue;
    float a = deg * 3.14159f / 180.0f;
    float dx = side * sinf(a), dy = cosf(a);
    float x0 = 50.0f + side * shoulderDx(stage), y0 = shoulderY(stage);
    for (int i = 0; i <= 8; i++) {
      float u = i / 8.0f;
      dot(g, t, x0 + dx * armLen(deg) * u, y0 + dy * armLen(deg) * u, (i == 8 ? MAEUM_HAND_R : MAEUM_ARM_R) + o, col);
    }
  }
}

}  // namespace detail

inline void drawCloud(Adafruit_GFX& g, int cx, int cy, int widthPx, uint8_t stage, uint8_t mood, bool feet = true,
                      int leftArm = ARM_DOWN, int rightArm = ARM_DOWN) {
  using namespace detail;
  if (stage < 1) stage = 1;
  if (stage > 3) stage = 3;
  const float sf = stageScale(stage);
  Xf t{cx, cy, widthPx * sf / 100.0f};

  const Blob* b; int n;
  if (stage == 1)      { b = STAGE1; n = sizeof(STAGE1) / sizeof(Blob); }
  else if (stage == 2) { b = STAGE2; n = sizeof(STAGE2) / sizeof(Blob); }
  else                 { b = STAGE3; n = sizeof(STAGE3) / sizeof(Blob); }

  const float outlineW = 2.6f;                                         // outline thickness in design units

  arms(g, t, stage, leftArm, rightArm, 0);                             // arm outlines, behind the body

  if (feet) {                                                          // two feet, drawn first so the body covers their top
    const float baseY = (stage == 1) ? 76.0f : (stage == 2 ? 75.0f : 77.0f);
    const float fy = baseY + 3.5f, fx = 12.0f;                         // foot center height and distance from the middle
    const float hw = 7.5f, hh = 5.5f;                                  // half width / half height of a foot
    for (int pass = 0; pass < 2; pass++) {
      const float o = (pass == 0) ? outlineW : 0.0f;                   // pass 0 = outline, pass 1 = cream
      const uint16_t col = (pass == 0) ? C_OUTLINE : C_CREAM;
      for (int side = -1; side <= 1; side += 2) {
        int w = t.R(hw + o), h = t.R(hh + o);
        g.fillRoundRect(t.X(50.0f + side * fx) - w, t.Y(fy) - h, 2 * w, 2 * h, h, col);
      }
    }
  }
  for (int i = 0; i < n; i++) g.fillCircle(t.X(b[i].x), t.Y(b[i].y), t.R(b[i].r + outlineW), C_OUTLINE);
  for (int i = 0; i < n; i++) g.fillCircle(t.X(b[i].x), t.Y(b[i].y), t.R(b[i].r), C_CREAM);

  arms(g, t, stage, leftArm, rightArm, 1);                             // arm fill, in front of the body
  if (stage == 1) curl(g, t, 46.0f, 15.0f);
  if (stage == 2) curl(g, t, 41.0f, 19.0f);

  float ex = (stage == 1) ? 14.0f : (stage == 2 ? 12.0f : 12.5f);
  float ey = (stage == 1) ? 50.0f : (stage == 2 ? 49.0f : 46.0f);
  float by = ey + (stage == 1 ? 8.0f : 7.0f);
  float bx = (stage == 1) ? 26.0f : (stage == 2 ? 23.5f : 25.5f);
  float br = (stage == 1) ? 8.0f : (stage == 2 ? 7.5f : 8.0f);
  dot(g, t, 50.0f - bx, by, br, C_BLUSH);
  dot(g, t, 50.0f + bx, by, br, C_BLUSH);
  eyes(g, t, stage, mood, ex, ey);

  float my = ey + (stage == 1 ? 8.0f : 7.0f);                            // small "w" mouth
  arc(g, t, 47.6f, my, 2.4f, 2.0f, +1, 1.0f, C_NAVY);
  arc(g, t, 52.4f, my, 2.4f, 2.0f, +1, 1.0f, C_NAVY);

  if (stage == 3) sparkle(g, t, 93.0f, 9.0f, 8.0f, 2.2f);
}

// pixel position of a hand (right = true for the cloud's right side on screen). Use it to attach props.
inline void armHand(int cx, int cy, int widthPx, uint8_t stage, bool right, int deg, int& hx, int& hy) {
  using namespace detail;
  if (stage < 1) stage = 1;
  if (stage > 3) stage = 3;
  Xf t{cx, cy, widthPx * stageScale(stage) / 100.0f};
  float a = deg * 3.14159f / 180.0f, side = right ? 1.0f : -1.0f;
  hx = t.X(50.0f + side * shoulderDx(stage) + side * sinf(a) * armLen(deg));
  hy = t.Y(shoulderY(stage) + cosf(a) * armLen(deg));
}

// small beanie on the cloud's right lobe (call right after drawCloud with the same cx, cy, widthPx, stage)
inline void drawHat(Adafruit_GFX& g, int cx, int cy, int widthPx, uint8_t stage, uint16_t color = C_ORANGE) {
  using namespace detail;
  if (stage < 1) stage = 1;
  if (stage > 3) stage = 3;
  Xf t{cx, cy, widthPx * stageScale(stage) / 100.0f};
  const float hx = (stage == 1) ? 61.0f : 63.0f, by = (stage == 1) ? 21.0f : (stage == 2 ? 9.0f : 8.0f);   // hat base (design units)
  for (int pass = 0; pass < 2; pass++) {
    const float o = (pass == 0) ? 2.0f : 0.0f;
    const uint16_t c1 = (pass == 0) ? C_OUTLINE : color, c2 = (pass == 0) ? C_OUTLINE : C_CREAM;
    int w = t.R(10.5f + o), h = t.R(11.0f + o);
    g.fillRoundRect(t.X(hx) - w, t.Y(by) - h - t.R(3.0f), 2 * w, 2 * h, w, c1);                      // dome
    int bw = t.R(14.5f + o), bh = t.R(2.8f + o);
    g.fillRoundRect(t.X(hx) - bw, t.Y(by) - bh, 2 * bw, 2 * bh + t.R(1.0f), bh, c1);                  // brim
    g.fillCircle(t.X(hx), t.Y(by) - 2 * h - t.R(0.5f), t.R(4.6f + o), c2);                            // pom-pom
  }
}

}  // namespace maeum
