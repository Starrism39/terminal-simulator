#pragma once

#include <pcap/pcap.h>

#include <atomic>

#include "airimgrecv.h"
#include "airtextrecv.h"

typedef void VPSMPcapHandler(unsigned char *param, const struct pcap_pkthdr *header, const unsigned char *pkt_data);

#define VPS2M_ETHERNET_CNT 40
#define VPS2M_ETHERNET_LEN 1024

typedef struct TAG_EthernetInfo {
  char Ethernet[VPS2M_ETHERNET_CNT][VPS2M_ETHERNET_LEN];
  unsigned int NetMask[VPS2M_ETHERNET_CNT];
  int EthernetCnt = -1;
} VPSEthernetInfo;

class VPS4MPcap : public vpskit::NonCopyable {
 public:
  static VPS4MPcap &instance() {
    static VPS4MPcap _instance;
    return _instance;
  }

  AirImgRecv *imgrev = nullptr;
  AirTextRecv *textrev = nullptr;

  bool init10g();
  bool initRev(int full_img_nb, int scan_img_nb);

  bool pcapQueryEthernet();
  int getEthernetCnt();

  void pcapSetHandler(VPSMPcapHandler *packet_handler);
  bool pcapInit(int index);

  void resetPcapEthHandle() { pcapClose(); }

  void pcapClose();

  pcap_t *getEthHandle() { return m_ethHandle; }

  VPSEthernetInfo getEthernetScanResult();

  void closeAll();
  int openCapture(const char *filter_exp, uint8_t *custom_param);
  void *recvPack(void *vdata, void *out_data);

 private:
  VPS4MPcap();
  ~VPS4MPcap();

  VPSEthernetInfo *m_info;
  pcap_t *volatile m_ethHandle;
  int volatile m_ethIndex;

  VPSMPcapHandler *m_packet_handler;
};

bool recvCheck(uint8_t *buf, int len);
