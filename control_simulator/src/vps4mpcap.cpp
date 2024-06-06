#include "vps4mpcap.h"

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <queue>

#include "airimgrecv.h"
#include "ethernet.h"
using namespace std;

// pcap open device错误
#define RTN_ERR_FIND_DEVICES (-3)
#define RTN_ERR_OPEN_ADPTER (-4)
#define RTN_ERR_DLT_EN10MB (-5)
#define RTN_OK (0)

VPS4MPcap::VPS4MPcap() {
  m_ethHandle = NULL;
  m_ethIndex = -1;
  m_info = new VPSEthernetInfo;
  m_packet_handler = NULL;
}

VPS4MPcap::~VPS4MPcap() {
  closeAll();
  if (imgrev != nullptr) {
    /* code */
    delete imgrev;
  }
}

void VPS4MPcap::closeAll() {
  if (m_info) {
    delete m_info;
    m_info = NULL;
  }
  pcapClose();
}

bool recvCheck(uint8_t *buf, int len)  // 16
{
  if (len > 1 && len < 2048) {
    uint8_t sum = 0;
    for (int i = 0; i < len - 1; i++) sum += buf[i];
    return sum == buf[len - 1];
  }
  return false;
}

// pcap回调，注意这里是跨线程调用
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data) {
  if (!pkt_data || header->len < 60 || header->len > 1514) return;

  if (header->len == 1083 && pkt_data[42] == 0x81) {
    if (recvCheck((uint8_t *)pkt_data + 42, 1041)) {
      //  process data
    }
  }
}

void *VPS4MPcap::recvPack(void *vdata, void *out_data) { return imgrev->imgRecv((uint8_t *)vdata, out_data); }

bool VPS4MPcap::initRev(int full_img_nb, int scan_img_nb) {
  imgrev = new AirImgRecv(full_img_nb, scan_img_nb);
  return imgrev->mallocMemoryPool();
}
bool VPS4MPcap::init10g() {
  pcapClose();
  pcapQueryEthernet();
  int cnt = getEthernetCnt();
  for (int i = 0; i < cnt; i++) {
    int res = strcmp(m_info->Ethernet[i], "eth0");
    if (res == 0) {
      // defalut callback handler
      pcapSetHandler(packet_handler);
      if (!pcapInit(i)) {
        resetPcapEthHandle();
        return false;
      }
      usleep(250 * 1000);
      return true;
    }
  }
  return false;
}

bool VPS4MPcap::pcapQueryEthernet() {
  m_info->EthernetCnt = 0;

  pcap_if_t *d;
  pcap_if_t *alldevs;
  char errbuf[PCAP_ERRBUF_SIZE];
  if (pcap_findalldevs(&alldevs, errbuf) == -1) {
    return false;
  }

  int inum = 0;
  for (d = alldevs; d; d = d->next) {
    if (d->description != NULL) {
      string description(d->description);
      if (description.find("virtual") != string::npos || description.find("Virtual") != string::npos ||
          description.find("wireless") != string::npos || description.find("Wireless") != string::npos)
        continue;
    }

    strcpy(m_info->Ethernet[inum], d->name);

    inum++;
  }
  m_info->EthernetCnt = inum;
  pcap_freealldevs(alldevs);

  return inum > 0;
}

int VPS4MPcap::getEthernetCnt() { return m_info->EthernetCnt; }

VPSEthernetInfo VPS4MPcap::getEthernetScanResult() { return *m_info; }

int VPS4MPcap::openCapture(const char *filter_exp, uint8_t *custom_param) {
  if (m_ethIndex < 0 || m_ethIndex > m_info->EthernetCnt - 1) {
    return RTN_ERR_FIND_DEVICES;
  }
  
  char errbuf[PCAP_ERRBUF_SIZE];  // errors buffer
  struct bpf_program fp;          // BPF filter

  // open device
  m_ethHandle = pcap_open_live(m_info->Ethernet[m_ethIndex], 1200, 1, 1000, errbuf);
  if (m_ethHandle == NULL) {
    fprintf(stderr, "can't open Ethernet: %s\n", errbuf);
    return 1;
  }
 
  // compile and set filter rules
  if (pcap_compile(m_ethHandle, &fp, filter_exp, 0, PCAP_NETMASK_UNKNOWN) == -1) {
    fprintf(stderr, "can't compile filter rules: %s\n", pcap_geterr(m_ethHandle));
    return 1;
  }
  if (pcap_setfilter(m_ethHandle, &fp) == -1) {
    fprintf(stderr, "can't set filter rules: %s\n", pcap_geterr(m_ethHandle));
    return 1;
  }
  
  // start capture
  pcap_loop(m_ethHandle, 0, m_packet_handler, custom_param);
  
  // close
  pcap_close(m_ethHandle);

  return RTN_OK;
}

void VPS4MPcap::pcapSetHandler(VPSMPcapHandler *packet_handler) { m_packet_handler = packet_handler; }

bool VPS4MPcap::pcapInit(int index) {
  if (m_ethHandle != NULL) {
    resetPcapEthHandle();
  }

  m_ethIndex = index;

  return true;
}

void VPS4MPcap::pcapClose() {
  if (m_ethHandle) {
    pcap_breakloop(m_ethHandle);
  }
}
