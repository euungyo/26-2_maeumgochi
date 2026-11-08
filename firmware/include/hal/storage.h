#pragma once

#include <stddef.h>

namespace hal {

// 자료형 구조는 호출자가 책임집니다.
// 구조체를 저장할 때는 버전 필드를 포함해 구조 변경에 대비하세요.
class Storage {
 public:
  virtual ~Storage() = default;

  virtual bool begin() = 0;
  virtual bool save(const char* key, const void* data, size_t size) = 0;
  // 저장된 크기가 size와 다르면 false를 반환하고 data를 건드리지 않습니다.
  virtual bool load(const char* key, void* data, size_t size) = 0;
  virtual bool exists(const char* key) = 0;
  virtual bool remove(const char* key) = 0;
};

}  // namespace hal