#include "udp.hpp"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

int main(int argc, char const *argv[]) {
  try {
    slip::Udp udp;
    if (argv[1][0] == 's') {
      std::string dest_ip(argv[2]);
      unsigned short dest_port = atoi(argv[3]), source_port = atoi(argv[4]);
      int n = atoi(argv[5]);
      for (int i = 0; i < n; ++i) {
        std::stringstream stream;
        stream << "message " << i;
        std::string message = stream.str();
        udp.send(dest_ip, dest_port, source_port, message);
        std::cout << "message " << i << " send" << std::endl;
      }
    } else if (argv[1][0] == 'l') {
      unsigned short listen_port = atoi(argv[2]);
      int n = atoi(argv[3]), count = 0;
      udp.add_listener(listen_port, [&count](std::string source_ip, unsigned short source_port, std::string message)->void{
        std::cout << "=====receive=====" << std::endl;
        std::cout << "source_ip: " << source_ip << std::endl;
        std::cout << "source_port: " << source_port << std::endl;
        std::cout << "message: " << message << std::endl;
        ++count;
      });
      while (count < n) {}
    }
  } catch (std::runtime_error e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
