#include "tcp.hpp"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>

int main(int argc, char const *argv[]) {
  try {
    slip::Tcp tcp;
    if (argv[1][0] == 's') {
      // send mode
      std::string dest_ip(argv[2]);
      unsigned short dest_port = atoi(argv[3]), source_port = atoi(argv[4]);
      int n = atoi(argv[5]);
      auto pcb = tcp.connect(dest_ip, dest_port, source_port);
      for (int i = 0; i < n; ++i) {
        std::cout << i << std::endl;
        std::stringstream stream;
        stream << "message " << i;
        std::string message = stream.str();
        pcb->send(message);
      }
      pcb->close();
    } else if (argv[1][0] == 'l') {
      // listen mode
      unsigned short source_port = atoi(argv[2]);
      auto pcb = tcp.listen(source_port);
      pcb->add_listener([](std::string data)->void {
        std::cout << "receive:" << std::endl;
        std::cout << data << std::endl;
      });
      while (pcb->state != slip::Tcp::CLOSED) {};
    }
  } catch (std::runtime_error e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
