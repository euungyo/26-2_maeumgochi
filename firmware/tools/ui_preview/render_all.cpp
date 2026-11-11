// render_all.cpp - draws every screen with a fake Adafruit_GFX on the PC and saves BMP files (no board needed).
// See docs/ui/README.md for the compile command.
#include <cstdio>
#include "ui/art/fishing_demo.h"

static void saveBmp(const Adafruit_GFX& g, const char* name) {
  char fn[96]; snprintf(fn, sizeof fn, "%s.bmp", name);
  FILE* f = fopen(fn, "wb"); if (!f) return;
  int w = g.W, h = g.H, rowBytes = (w * 3 + 3) / 4 * 4, size = 54 + rowBytes * h;
  unsigned char hd[54] = {0}; hd[0] = (unsigned char)66; hd[1] = (unsigned char)77;
  hd[2] = size; hd[3] = size >> 8; hd[4] = size >> 16; hd[10] = 54; hd[14] = 40;
  hd[18] = w; hd[19] = w >> 8; hd[22] = h; hd[23] = h >> 8; hd[26] = 1; hd[28] = 24;
  fwrite(hd, 1, 54, f);
  for (int y = h - 1; y >= 0; y--) {
    for (int x = 0; x < w; x++) {
      uint16_t p = g.buf[y * w + x];
      unsigned char bgr[3] = {(unsigned char)((p & 31) * 255 / 31), (unsigned char)(((p >> 5) & 63) * 255 / 63), (unsigned char)(((p >> 11) & 31) * 255 / 31)};
      fwrite(bgr, 1, 3, f);
    }
    for (int k = w * 3; k < rowBytes; k++) fputc(0, f);
  }
  fclose(f);
}
#define SHOT(name, call) { Adafruit_GFX g(240, 240); call; saveBmp(g, name); }
int main() {
  SHOT("screen01_main", maeum_screen::mainScreen(g))           SHOT("screen02_life_menu", maeum_screen::lifeMenuScreen(g))
  SHOT("screen03_char_menu", maeum_screen::charMenuScreen(g))  SHOT("screen04_mission_list", maeum_screen::missionListScreen(g))
  SHOT("screen05_mission_detail", maeum_screen::missionDetailScreen(g)) SHOT("screen06_checkin", maeum_screen::checkinScreen(g))
  SHOT("screen07_confirm", maeum_screen::confirmScreen(g))     SHOT("screen08_shop", maeum_screen::shopListScreen(g))
  SHOT("screen09_item", maeum_screen::itemDetailScreen(g))     SHOT("screen10_bag", maeum_screen::bagScreen(g))
  SHOT("screen11_growth", maeum_screen::growthScreen(g))       SHOT("screen12_growth_up", maeum_screen::growthUpScreen(g))
  SHOT("screen13_breakfast", maeum_screen::mealBreakfast(g))   SHOT("screen14_lunch", maeum_screen::mealLunch(g))
  SHOT("screen15_dinner", maeum_screen::mealDinner(g))         SHOT("screen16_sleep", maeum_screen::sleepScreen(g))
  SHOT("screen17_record", maeum_screen::recordScreen(g))       SHOT("screen18_low_battery", maeum_screen::lowBatteryScreen(g))
  SHOT("screen19_saved", maeum_screen::savedScreen(g))
  SHOT("mission_make_bed", maeum_scene::makeBedScene(g))       SHOT("mission_drink_water", maeum_scene::drinkWaterScene(g))
  SHOT("mission_wash", maeum_scene::washScene(g))              SHOT("mission_stretch", maeum_scene::stretchScene(g))
  SHOT("mission_air_out", maeum_scene::airOutScene(g))         SHOT("mission_walk", maeum_scene::walkScene(g))
  SHOT("mission_shower", maeum_scene::showerScene(g))          SHOT("mission_clean", maeum_scene::cleanScene(g))
  SHOT("mission_groceries", maeum_scene::groceriesScene(g))    SHOT("mission_cook", maeum_scene::cookScene(g))
  SHOT("mission_study", maeum_scene::studyScene(g))            SHOT("mission_plant", maeum_scene::waterPlantScene(g))
  SHOT("fishing1_wait", maeum_fish::fishScene(g, maeum_fish::WAIT, 0.0f)) SHOT("fishing2_nibble", maeum_fish::fishScene(g, maeum_fish::NIBBLE, 0.1f))
  SHOT("fishing3_bite", maeum_fish::fishScene(g, maeum_fish::BITE, 0.5f)) SHOT("fishing4_pull", maeum_fish::fishScene(g, maeum_fish::PULL, 0.0f))
  SHOT("fishing5_caught", maeum_fish::fishScene(g, maeum_fish::CAUGHT, 0.0f)) SHOT("fishing6_miss", maeum_fish::fishScene(g, maeum_fish::MISS, 0.0f))

  puts("saved 37 BMP files in the current folder");
  return 0;
}
