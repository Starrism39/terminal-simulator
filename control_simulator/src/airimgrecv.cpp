#include "airimgrecv.h"

#include <cstring>
#include <iostream>
AirImgRecv::AirImgRecv(int full_img_nb, int scan_img_nb) {
  mem_pool_map_[AIRMODE_FULL] = new ImgMemoryManager(full_img_nb, 15360, 640);
  mem_pool_map_[AIRMODE_SCAN] = new ImgMemoryManager(scan_img_nb, 2048, 1892);
  uint16_t scan_pkts[4] = {3783, 3775, 1891, 1887};
  uint16_t scan_size[4][2] = {{2048, 1892}, {2048, 1888}, {1024, 1892}, {1024, 1888}};
  for (size_t i = 0; i < 4; i++) {
    frameInfo_[i].pkts = scan_pkts[i];
    frameInfo_[i].w = scan_size[i][0];
    frameInfo_[i].h = scan_size[i][1];
  }
}

AirImgRecv::~AirImgRecv() {
  for (auto it = mem_pool_map_.begin(); it != mem_pool_map_.end(); ++it) {
    delete it->second;
  }
  mem_pool_map_.clear();
}

ScanAreaColor AirImgRecv::getAreaColor(uint16_t idx) {
  if (idx == 71) {
    return ScanAreaColor::scanAreaOrig;
  } else if (idx % 8 <= 6 && idx / 9 < 7) {
    return ScanAreaColor::scanAreaYellow;
  } else if (idx % 8 == 7) {
    return ScanAreaColor::scanAreaGreen;
  } else if (idx / 8 == 8) {
    return ScanAreaColor::scanAreaBlue;
  }
}

bool AirImgRecv::isLastPacket(uint8_t mode,uint16_t line,uint16_t pkt) {
  // if (!data) return false;

  // uint8_t mode = data[1];

  // uint8_t quad = data[3];

  // uint16_t line = vpskit::transEnd(*(uint16_t *)((data + 4)));
  // uint16_t pkt = vpskit::transEnd(*(uint16_t *)((data + 6)));

  if (mode == AIRMODE_FULL) 
  {
    
    
        if (pkt == 14 && ((line + 1) % full_h_ == 0 || line == 17023)) {
          /* code */
          return true;
        } else {
          return false;
        }
      
      
  } else if (mode == AIRMODE_SCAN) {
    if (pkt == frameInfo_[getAreaColor(line)].pkts) {
      /* code */
      return true;
    } else {
      return false;
    }
  }

  return false;
}
bool AirImgRecv::mallocMemoryPool() {
  for (auto it = mem_pool_map_.begin(); it != mem_pool_map_.end(); ++it) {
    if (it->second->mallocMemoryPool() == false) return false;
  }
  return true;
}
void AirImgRecv::releaseMemory(AirImg *img) { mem_pool_map_[img->mode]->setFree(*(img->data - 1)); }

void *AirImgRecv::imgRecv(uint8_t *data, void *out_data) {
  if (!shouldRecv_ || !data) return nullptr;

  if (data[1] == AIRMODE_FULL || data[1] == AIRMODE_SCAN) {
    int offset = 0;
    // uint8_t fun = data[0];
    uint8_t mode = data[1];
    uint8_t frame = data[2];
    uint8_t quad = data[3];
    uint16_t line = vpskit::transEnd(*(uint16_t *)((data + 4)));
    uint16_t pkt = vpskit::transEnd(*(uint16_t *)((data + 6)));

    if ((quad == 1 || quad== 3) && mode == AIRMODE_FULL)
    {
      line=17023 - line;
    }
    
    if (mem_pool_map_.find(mode) == mem_pool_map_.end()) {
      printf("mode memory pool not found\n");
      return nullptr;
    }
    uint8_t *dst = nullptr;
    if (mode == AIRMODE_FULL) {
      // full mode
      // if (quad == 1 || quad == 3) {
      //   if (line > 16639)
      //   {
      //     offset = (639-((line) % full_h_)) * full_w_ + pkt * 1024;  //////////  the img received may top bottom change
      //   }else{
      //     offset = (383 - ((line) % full_h_)) * full_w_ + pkt * 1024; /////
      //   } 
      // } else if (quad == 0 || quad == 2) {
      //   offset = (line % full_h_) * full_w_ + pkt * 1024;    /////////////////////
      // }
      offset = (line % full_h_) * full_w_ + pkt * 1024;
      dst = mem_pool_map_[mode]->getFreeMem(full_w_ * full_h_) + offset;
    } else if (mode == AIRMODE_SCAN) {
      offset = pkt * 1024;                                                   //////////////
      dst = mem_pool_map_[mode]->getFreeMem(scan_w_ * scan_h_) + offset;      
    }
    if (dst == nullptr) {
      // std::cout<<__FILE__<<__LINE__<<"|"<<line<<std::endl;
      printf("can't get free mem\n");
      return nullptr;
    }
    memcpy(dst, data + 16, 1024);

    if (isLastPacket(mode,line,pkt)) {
      // std::cout<<__FILE__<<__LINE__<<"|"<<line<<std::endl;
      AirImg *img = new AirImg;
      img->mode = mode;
      img->frame_id = frame;
      img->quadrant_idx = quad;
      if (mode == AIRMODE_FULL) {
        img->w = full_w_;
        
        img->h = line % 640 + 1;
        
        img->idx = line / full_h_;
        img->data = mem_pool_map_[mode]->getFreeMem(full_w_ * full_h_);
        if (quad == 0 || quad == 1) {
          img->winX = 0;
        } else if (quad == 2 || quad == 3) {
          img->winX = 15360;
        }
        if (quad == 0 || quad == 2) {
          img->winY = line - img->h + 1;
        } else if (quad == 1 || quad == 3) {
          img->winY =17024 + 17023 - line; //////////  ////////
        }
      } else if (mode == AIRMODE_SCAN) {
        auto frameInfo = frameInfo_[getAreaColor(line)];
        img->w = frameInfo.w;
        img->h = frameInfo.h;
        img->idx = line;

        img->winX = line % 8 * 2048;
        if (quad == 0 || quad == 1) {
          img->winX += 0;
        } else if (quad == 2 || quad == 3) {
          img->winX += 15360;
        }

        if (quad == 0 || quad == 2) {
          img->winY = line / 8 * 1892;
        } else if (quad == 1 || quad == 3) {
          img->winY = 17024 + 17024 - line / 8 * 1892 - img->h;
        }

        img->data = mem_pool_map_[mode]->getFreeMem(scan_w_ * scan_h_);
      }
      mem_pool_map_[mode]->setBusy();
      return reinterpret_cast<void *>(img);
    }
  }
  return nullptr;
}
