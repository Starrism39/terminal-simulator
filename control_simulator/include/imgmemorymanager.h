#pragma once

#include <unistd.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <stdexcept>

#include "ethernet.h"

#pragma once

namespace vpskit {

class NonCopyable {
 protected:
  NonCopyable() = default;
  ~NonCopyable() = default;

 public:
  NonCopyable(const NonCopyable &) = delete;             // Copy construct
  NonCopyable(NonCopyable &&) = delete;                  // Move construct
  NonCopyable &operator=(const NonCopyable &) = delete;  // Copy assign
  NonCopyable &operator=(NonCopyable &&) = delete;       // Move assign
};
}  // namespace vpskit

class ImgMemoryManager : public vpskit::NonCopyable {
 public:
  ImgMemoryManager(int max_unit_nb, uint16_t w, uint16_t h) : img_nums_(max_unit_nb), unit_w_(w), unit_h_(h) {
    mem_pool_size_ = static_cast<size_t>(img_nums_) * static_cast<size_t>(unit_w_) * static_cast<size_t>(unit_h_);
  }
  ~ImgMemoryManager();

  bool mallocMemoryPool();
  void releaseMemory();

  void setImgSize(uint16_t w, uint16_t h) {
    unit_w_ = w;
    unit_h_ = h;
  }
  unsigned char *getFreeMem(int offset);

  void setBusy();
  void setFree(int8_t point);

 private:
  unsigned char *m_pool_ = nullptr;

  int img_nums_;
  uint16_t unit_w_ = 0;
  uint16_t unit_h_ = 0;

  size_t mem_pool_size_;
  std::atomic<bool> *busy_flags_;
  int8_t using_point_ = -1;
};
