#pragma once

#include <Preferences.h>

#include "hal/storage.h"

namespace drivers {

// ESP32 NVS(Preferences)에 키별로 바이트 덩어리를 저장합니다.
// 키 이름은 15자 이하여야 합니다. 같은 키에 쓰는 도중 전원이 꺼져도 이전 값이나 새 값 중 하나가 남습니다.
class NvsStorage : public hal::Storage {
 public:
  explicit NvsStorage(const char* namespaceName = "maeumgochi");

  bool begin() override;
  bool save(const char* key, const void* data, size_t size) override;
  // 저장된 크기가 size와 다르면 false를 반환하고 data를 건드리지 않습니다.
  bool load(const char* key, void* data, size_t size) override;
  bool exists(const char* key) override;
  bool remove(const char* key) override;

 private:
  const char* namespace_;
  Preferences prefs_;
  bool open_ = false;
};

}  // namespace drivers
