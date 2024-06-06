#pragma once

#include <atomic>

#include "imgmemorymanager.h"

struct Airboxes {
//   uint8_t frame_head;
//   uint8_t UID;
//   uint8_t win_id;
//   uint8_t img_mode;
//   uint32_t win_coords;

//   uint8_t track_mode;
//   uint8_t work_mode;  
//   uint8_t obj_num;
//   uint8_t obj;  
//   uint8_t frame_end;  
  uint8_t *data;
};



class AirTextRecv {
 public:
  AirTextRecv(int text_nb);
  ~AirTextRecv();
  void *TextRecv(uint8_t *data, void *out_data);
  bool mallocMemoryPool();
  void releaseMemory(Airboxes *img);
  void startRecv() { shouldRecv_ = true; }
  void stopRecv() { shouldRecv_ = false; }

 private:
  bool shouldRecv_ = true;
  int w_ = 1024;
  int h_ = 1; 
  ImgMemoryManager*  mem_pool_;
};
