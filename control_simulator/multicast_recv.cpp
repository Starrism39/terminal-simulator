#include <iomanip>
#include <iostream>

#include "udp_operation.h"

void printMessageHex(const char *message, size_t length) {
  std::cout << "Message in hexadecimal format:" << std::endl;
  for (size_t i = 0; i < length; ++i) {
    std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(static_cast<uint8_t>(message[i]))
              << " ";
  }
  std::cout << std::endl;
}

int main(int argc, char **argv) {
  std::string address = *(argv + 1);
  int port = std::atoi(*(argv + 2));

  UDPOperation server(address.c_str(), port, "eth0");
  server.create_client();

  char buffer[1024];

  while(true){
    int n = server.recv_buffer(reinterpret_cast<char *>(buffer), 1024);
    if(n > 20){
        printMessageHex(buffer, n);
    }
    
  }
}