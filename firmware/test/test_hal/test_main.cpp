#include <unity.h>

#include <map>
#include <string>
#include <vector>

#include "hal/buttons.h"
#include "hal/display.h"
#include "hal/haptic.h"
#include "hal/light_sensor.h"
#include "hal/storage.h"

namespace {

class FakeStorage : public hal::Storage {
 public:
  bool begin() override { return true; }
  bool save(const char* key, const void* data, size_t size) override {
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    items_[key] = std::vector<unsigned char>(bytes, bytes + size);
    return true;
  }
  bool load(const char* key, void* data, size_t size) override {
    auto it = items_.find(key);
    if (it == items_.end() || it->second.size() != size) {
      return false;
    }
    unsigned char* out = static_cast<unsigned char*>(data);
    for (size_t i = 0; i < size; ++i) {
      out[i] = it->second[i];
    }
    return true;
  }
  bool exists(const char* key) override { return items_.count(key) > 0; }
  bool remove(const char* key) override { return items_.erase(key) > 0; }

 private:
  std::map<std::string, std::vector<unsigned char>> items_;
};

class MissingLightSensor : public hal::LightSensor {
 public:
  bool begin() override { return false; }
  void update() override {}
  bool isValid() const override { return false; }
  hal::LightReading latest() const override { return {0.0f, 0.0f, false}; }
};

void test_storage_round_trip() {
  FakeStorage storage;
  const int saved = 1234;
  int loaded = 0;

  TEST_ASSERT_TRUE(storage.save("coins", &saved, sizeof(saved)));
  TEST_ASSERT_TRUE(storage.exists("coins"));
  TEST_ASSERT_TRUE(storage.load("coins", &loaded, sizeof(loaded)));
  TEST_ASSERT_EQUAL_INT(1234, loaded);
}

void test_storage_rejects_size_mismatch() {
  FakeStorage storage;
  const int saved = 7;
  long long wrongSize = 99;

  storage.save("coins", &saved, sizeof(saved));
  TEST_ASSERT_FALSE(storage.load("coins", &wrongSize, sizeof(wrongSize)));
  TEST_ASSERT_EQUAL_INT(99, static_cast<int>(wrongSize));
}

void test_storage_missing_key() {
  FakeStorage storage;
  int value = 0;

  TEST_ASSERT_FALSE(storage.exists("none"));
  TEST_ASSERT_FALSE(storage.load("none", &value, sizeof(value)));
}

void test_missing_light_sensor_is_reported_invalid() {
  MissingLightSensor sensor;

  TEST_ASSERT_FALSE(sensor.begin());
  TEST_ASSERT_FALSE(sensor.isValid());
  TEST_ASSERT_FALSE(sensor.latest().valid);
}

void test_rgb565_conversion() {
  TEST_ASSERT_EQUAL_HEX16(0xFFFF, hal::rgb565(255, 255, 255));
  TEST_ASSERT_EQUAL_HEX16(0xF800, hal::rgb565(255, 0, 0));
  TEST_ASSERT_EQUAL_HEX16(0x0000, hal::rgb565(0, 0, 0));
}

}  // namespace

extern "C" void setUp(void) {}
extern "C" void tearDown(void) {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(test_storage_round_trip);
  RUN_TEST(test_storage_rejects_size_mismatch);
  RUN_TEST(test_storage_missing_key);
  RUN_TEST(test_missing_light_sensor_is_reported_invalid);
  RUN_TEST(test_rgb565_conversion);
  return UNITY_END();
}