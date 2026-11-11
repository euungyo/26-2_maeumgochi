// ko_text.h - draws UTF-8 text (Korean, English, digits) with the generated bitmap font in ko_font.h.
//   maeum::ko::draw(g, x, y, "미션", color);            // 16 px high
//   maeum::ko::draw(g, x, y, "미션", color, 2);         // 32 px high (each pixel doubled)
//   maeum::ko::width("미션", 1)                         // text width in pixels
#pragma once
#include <Adafruit_GFX.h>
#include "ko_font.h"

namespace maeum { namespace ko {

inline const ko_data::Glyph* find(uint16_t cp) {
  int lo = 0, hi = ko_data::GLYPH_COUNT - 1;
  while (lo <= hi) {
    int mid = (lo + hi) / 2;
    uint16_t c = ko_data::GLYPHS[mid].cp;
    if (c == cp) return &ko_data::GLYPHS[mid];
    if (c < cp) lo = mid + 1; else hi = mid - 1;
  }
  return nullptr;
}

inline uint16_t next(const char*& s) {                    // read one UTF-8 character (up to 3 bytes)
  uint8_t c = (uint8_t)*s++;
  if (c < 0x80) return c;
  if ((c & 0xE0) == 0xC0) { uint16_t r = (c & 0x1F) << 6; r |= (*s++ & 0x3F); return r; }
  uint16_t r = (c & 0x0F) << 12; r |= (*s++ & 0x3F) << 6; r |= (*s++ & 0x3F); return r;
}

inline int width(const char* s, int scale = 1) {
  int w = 0;
  while (*s) { const ko_data::Glyph* gl = find(next(s)); w += (gl ? gl->w : 8) * scale; }
  return w;
}

inline void draw(Adafruit_GFX& g, int x, int y, const char* s, uint16_t color, int scale = 1) {
  while (*s) {
    const ko_data::Glyph* gl = find(next(s));
    if (!gl) { x += 8 * scale; continue; }
    const uint8_t* bits = ko_data::BITS + gl->off;
    if (scale == 1) {
      g.drawBitmap(x, y, bits, gl->w, 16, color);
    } else {
      int bpr = gl->w / 8;
      for (int row = 0; row < 16; row++)
        for (int col = 0; col < gl->w; col++)
          if (bits[row * bpr + col / 8] & (0x80 >> (col & 7))) g.fillRect(x + col * scale, y + row * scale, scale, scale, color);
    }
    x += gl->w * scale;
  }
}

inline void drawCentered(Adafruit_GFX& g, int cx, int y, const char* s, uint16_t color, int scale = 1) { draw(g, cx - width(s, scale) / 2, y, s, color, scale); }
inline void drawRight(Adafruit_GFX& g, int rx, int y, const char* s, uint16_t color, int scale = 1) { draw(g, rx - width(s, scale), y, s, color, scale); }

}}  // namespace maeum::ko
