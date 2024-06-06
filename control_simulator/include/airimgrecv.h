#pragma once

#include <atomic>
#include <map>

#include "imgmemorymanager.h"

#define AIRMODE_FULL 0x00
#define AIRMODE_WIN 0x05
#define AIRMODE_SCAN 0x06

struct AirImgInfo {
  uint16_t pkts;
  int w;
  int h;
};

struct AirImg {
  uint8_t mode;
  uint8_t frame_id;
  int w;
  int h;
  uint8_t quadrant_idx;
  uint16_t idx;  // pkt
  uint8_t *data;
  uint16_t winX;  // winX
  uint16_t winY;  // winY
  /* data */
};

enum ScanAreaColor { scanAreaYellow = 0, scanAreaBlue, scanAreaGreen, scanAreaOrig };

class AirImgRecv {
 public:
  AirImgRecv(int full_img_nb, int scan_img_nb);
  ~AirImgRecv();
  void *imgRecv(uint8_t *data, void *out_data);
  bool mallocMemoryPool();
  void releaseMemory(AirImg *img);
  void startRecv() { shouldRecv_ = true; }
  void stopRecv() { shouldRecv_ = false; }

 private:
  bool isLastPacket(uint8_t mode,uint16_t line,uint16_t pkt);
  ScanAreaColor getAreaColor(uint16_t idx);  // get  area
  bool shouldRecv_ = true;
  int full_w_ = 15360;
  int full_h_ = 640;

  int scan_w_ = 2048;
  int scan_h_ = 1892;

  AirImgInfo frameInfo_[4];
  std::map<uint8_t, ImgMemoryManager *> mem_pool_map_;
};
