#include "imgmemorymanager.h"

ImgMemoryManager::~ImgMemoryManager() { releaseMemory(); }

bool ImgMemoryManager::mallocMemoryPool() {
  if (unit_w_ == 0 || unit_h_ == 0) {
    throw std::length_error("Not set img size");
    return false;
  }
  img_nums_ = mem_pool_size_ / static_cast<uint64_t>(unit_w_ * unit_h_);
  m_pool_ = new unsigned char[mem_pool_size_ + img_nums_];
  busy_flags_ = new std::atomic<bool>[img_nums_];
  for (int i = 0; i < img_nums_; i++) {
    busy_flags_[i].store(false);
  }
  return true;
}

void ImgMemoryManager::setBusy() {
  busy_flags_[using_point_].store(true);
  using_point_ = -1;
}
void ImgMemoryManager::setFree(int8_t point) { busy_flags_[point].store(false); }

unsigned char *ImgMemoryManager::getFreeMem(int offset) {
  if (m_pool_ == nullptr) return nullptr;

  if (using_point_ == -2)  // no free
    return nullptr;
  else if (using_point_ < img_nums_ && using_point_ != -1) {  // lock
    return m_pool_ + (offset + 1) * using_point_ + 1;
  } else if (using_point_ == -1) {
    for (int i = 0; i < img_nums_; i++) {
      if (!busy_flags_[i].load()) {
        using_point_ = i;
        *(m_pool_ + static_cast<uint64_t>((offset + 1)) * static_cast<uint64_t>(using_point_)) = using_point_;
        return m_pool_ + (offset + 1) * using_point_ + 1;
      }
    }
    using_point_ = -2;
  }
  return nullptr;
}

void ImgMemoryManager::releaseMemory() {
  if (m_pool_) {
    delete[] m_pool_;
    m_pool_ = nullptr;
  }
  delete[] busy_flags_;
}
