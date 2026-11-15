// Host-side stand-in for Adafruit_GFX (same method names) so the drawing code can be tested on a computer.
#pragma once
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <cmath>
#include <cstdio>
#include <algorithm>
class Adafruit_GFX {
 public:
  Adafruit_GFX(int w, int h) : W(w), H(h), buf(w * h, 0) {}
  virtual ~Adafruit_GFX() {}
  void drawPixel(int x, int y, uint16_t c) { if (x >= 0 && y >= 0 && x < W && y < H) buf[y * W + x] = c; }
  void fillScreen(uint16_t c) { std::fill(buf.begin(), buf.end(), c); }
  void drawFastHLine(int x, int y, int w, uint16_t c) { for (int i = 0; i < w; i++) drawPixel(x + i, y, c); }
  void drawFastVLine(int x, int y, int h, uint16_t c) { for (int j = 0; j < h; j++) drawPixel(x, y + j, c); }
  void drawLine(int x0, int y0, int x1, int y1, uint16_t c) {
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1, dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1, err = dx + dy;
    while (true) { drawPixel(x0, y0, c); if (x0 == x1 && y0 == y1) break; int e2 = 2 * err; if (e2 >= dy) { err += dy; x0 += sx; } if (e2 <= dx) { err += dx; y0 += sy; } }
  }
  void drawBitmap(int x, int y, const uint8_t* bm, int w, int h, uint16_t c) {   // 1-bit, MSB first, like Adafruit_GFX
    int bpr = (w + 7) / 8;
    for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) if (bm[j * bpr + i / 8] & (0x80 >> (i & 7))) drawPixel(x + i, y + j, c);
  }
  void fillRect(int x, int y, int w, int h, uint16_t c) { for (int j = 0; j < h; j++) drawFastHLine(x, y + j, w, c); }
  void fillCircle(int x0, int y0, int r, uint16_t c) { for (int dy = -r; dy <= r; dy++) { int dx = (int)std::floor(std::sqrt((double)r * r - (double)dy * dy) + 0.5); drawFastHLine(x0 - dx, y0 + dy, 2 * dx + 1, c); } }
  void fillRoundRect(int x, int y, int w, int h, int r, uint16_t c) {
    for (int j = 0; j < h; j++) for (int i = 0; i < w; i++) {
      int cx = std::min(std::max(i, r), w - 1 - r), cy = std::min(std::max(j, r), h - 1 - r);
      if ((i - cx) * (i - cx) + (j - cy) * (j - cy) <= r * r) drawPixel(x + i, y + j, c);
    }
  }
  void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, uint16_t c) {
    int minx = std::min({x0, x1, x2}), maxx = std::max({x0, x1, x2}), miny = std::min({y0, y1, y2}), maxy = std::max({y0, y1, y2});
    auto e = [](int ax, int ay, int bx, int by, int px, int py) { return (bx - ax) * (py - ay) - (by - ay) * (px - ax); };
    for (int y = miny; y <= maxy; y++) for (int x = minx; x <= maxx; x++) {
      int a = e(x0, y0, x1, y1, x, y), b = e(x1, y1, x2, y2, x, y), d = e(x2, y2, x0, y0, x, y);
      if ((a >= 0 && b >= 0 && d >= 0) || (a <= 0 && b <= 0 && d <= 0)) drawPixel(x, y, c);
    }
  }
  void setTextColor(uint16_t c) { tc = c; }
  void setTextSize(uint8_t s) { ts = s; }
  void setCursor(int x, int y) { cx = x; cy = y; }
  void print(const char* s) { texts.push_back({cx, cy, (int)ts, tc, std::string(s)}); cx += 6 * ts * (int)strlen(s); }
  struct Txt { int x, y, size; uint16_t color; std::string s; };
  int W, H; std::vector<uint16_t> buf; std::vector<Txt> texts; uint16_t tc = 0xFFFF; uint8_t ts = 1; int cx = 0, cy = 0;
};
