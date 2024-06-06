#include "airtextrecv.h"

#include <cstring>
#include <iostream>

AirTextRecv::AirTextRecv(int text_nb) {
    mem_pool_=new ImgMemoryManager(text_nb,1024,1);
}

AirTextRecv::~AirTextRecv() {
    delete mem_pool_;
}

bool AirTextRecv::mallocMemoryPool() {
if (mem_pool_->mallocMemoryPool() == false) return false;
  return true;
}

void AirTextRecv::releaseMemory(Airboxes *text) { mem_pool_->setFree(*(text->data - 1)); }

void *AirTextRecv::TextRecv(uint8_t *data, void *out_data)
{
    if (!shouldRecv_ || !data) return nullptr;
    int offset = 0;
    uint8_t *dst = nullptr;
    uint8_t obj_num = data[19];
    int plen= 20 + 16 * obj_num + 1;
    // offset = ;
    dst = mem_pool_->getFreeMem(w_ * h_);

    memcpy(dst, data, plen);

    Airboxes *text = new Airboxes;
    
    text->data = mem_pool_->getFreeMem(w_ * h_);
    mem_pool_->setBusy();
    return reinterpret_cast<void *>(text);

    // return nullptr;
}

