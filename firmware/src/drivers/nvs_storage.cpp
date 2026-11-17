#include "drivers/nvs_storage.h"

namespace drivers {

NvsStorage::NvsStorage(const char* namespaceName) : namespace_(namespaceName) {}

bool NvsStorage::begin() {
  if (!open_) {
    open_ = prefs_.begin(namespace_, /*readOnly=*/false);
  }
  return open_;
}

bool NvsStorage::save(const char* key, const void* data, size_t size) {
  return open_ && prefs_.putBytes(key, data, size) == size;
}

bool NvsStorage::load(const char* key, void* data, size_t size) {
  if (!open_ || prefs_.getBytesLength(key) != size) {
    return false;
  }
  return prefs_.getBytes(key, data, size) == size;
}

bool NvsStorage::exists(const char* key) { return open_ && prefs_.isKey(key); }

bool NvsStorage::remove(const char* key) { return open_ && prefs_.remove(key); }

}  // namespace drivers
