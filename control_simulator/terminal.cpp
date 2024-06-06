#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>  // OpenCV头文件
#include <queue>
#include <string>
#include <thread>

#include "opencv2/opencv.hpp"
#include "vps4mpcap.h"
#include "udp_operation.h"

void printMessageHex(const char *message, size_t length) {
  std::cout << "Message in hexadecimal format:" << std::endl;
  for (size_t i = 0; i < length; ++i) {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<uint8_t>(message[i]))
              << " ";
  }
  std::cout << std::endl;
}

static void m_packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data) {
  void *out_data = nullptr;
  if (!pkt_data || header->len < 19 || header->len > 1514) return;
  // std::cout<<int(pkt_data[42])<<std::endl;
  if (pkt_data[42] == 0x7e) {
    // std::cout<<"sad"<<std::endl;
    out_data = (uint8_t *)pkt_data + 42;
    if (out_data) {
      reinterpret_cast<std::queue<void *> *>(param)->push(out_data);
      // std::cout<<"param:"<<(void*)param<<std::endl;
    }
  }
}

void receive(std::queue<void *> capbuffer_, int port) {
  if (!VPS4MPcap::instance().init10g()) {
    /* code */
    printf("init pcap error.");
    throw std::runtime_error("init pcap rev error.");
  }

  std::cout << "start pcap running" << std::endl;
  VPS4MPcap::instance().pcapSetHandler(&m_packet_handler);
  std::thread([&]() {
    VPS4MPcap::instance().openCapture(("udp and port " + std::to_string(port)).c_str(),
                                      reinterpret_cast<uint8_t *>(&(capbuffer_)));
  }).detach();

  std::cout << "start pcap running" << std::endl;
  int i = 0;
  //std::cout << "capbuffer_:" << &capbuffer_ << std::endl;
  while (true) {
    if (!capbuffer_.empty()) {
      uint8_t *data = reinterpret_cast<uint8_t *>(capbuffer_.front());
      capbuffer_.pop();
      if (data) {
        uint8_t obj_num = data[19];
        size_t len = int(obj_num) * 16 + 21;
        printMessageHex(reinterpret_cast<char *>(data), len);
        i++;
      }
    }
  }
}

int main(int argc, char **argv) {
  std::queue<void *> capbuffer_;

  int port = atoi(argv[1]);
  std::string multicast_address = *(argv + 2);
  int multicast_port = std::atoi(*(argv + 3));

  UDPOperation server(multicast_address.c_str(), multicast_port, "eth0");
  server.create_server();

  std::thread informReceive(receive, std::ref(capbuffer_), port);
  informReceive.detach();

  char *rawData0 = "\xFF";
  char *rawData1 = "\x7E\x19\x03\x01\x01\x40\x00\xC8\x00\x28\x00\x3C\xFF\xFF\x01\xE7";  // 手动 win=1 x=320 y=200 w=40 h=60
  char *rawData2 = "\x7E\x19\x04\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xE7";  // 退出KCF win=1
  char *rawData3 = "\x7E\x19\x03\x01\x01\x40\x01\x9A\x00\x28\x00\x3C\x00\x02\x02\xE7";  // 自动 win=1 id=2
  char *rawData4 = "\x7E\x19\x05\x01\x01\x0A\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xE7";  // 跟踪微调 win=1 上移10个像素点
  char *rawData5 = "\x7E\x19\x05\x01\x03\x0A\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xE7";  // 跟踪微调 win=1 左移10个像素点
  char *rawData6 = "\x7E\x19\x04\x01\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xE7";  // 退出KCF win=1

  std::this_thread::sleep_for(std::chrono::seconds(1));
  server.send_buffer(reinterpret_cast<char *>(rawData0), 1);
  std::this_thread::sleep_for(std::chrono::seconds(8));

  // 发送 rawData1
  server.send_buffer(reinterpret_cast<char *>(rawData1), 16);
  std::cout << "message len: 16" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));

  // 发送 rawData2
  server.send_buffer(reinterpret_cast<char *>(rawData2), 16);
  std::cout << "message len: 16" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));

  // 发送 rawData3
  server.send_buffer(reinterpret_cast<char *>(rawData3), 16);
  std::cout << "message len: 16" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(5));

  // 发送 rawData4
  server.send_buffer(reinterpret_cast<char *>(rawData4), 16);
  std::cout << "message len: 16" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));

  // 发送 rawData5
  server.send_buffer(reinterpret_cast<char *>(rawData5), 16);
  std::cout << "message len: 16" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // 发送 rawData6
  server.send_buffer(reinterpret_cast<char *>(rawData6), 16);
  std::cout << "message len: 16" << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(3));

  while (1) {
  }


  return 0;
}