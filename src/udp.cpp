#include "udp.hpp"
#include "utils.hpp"
#include <cctype>
#include <cstring> //memset
#include <stdexcept>
#include <sys/socket.h>  //for socket ofcourse
#include <unistd.h>
#include <stdlib.h> //for exit(0);
#include <errno.h> //For errno - the error number
#include <netinet/udp.h> //Provides declarations for tcp header
#include <netinet/ip.h>  //Provides declarations for ip header
#include <netinet/in.h>
#include <arpa/inet.h>

#define DATAGRAM_MAX_LEN 4096

#include <iostream>

slip::Udp::Udp() {
  _finish = false;
  _socketfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
  if (_socketfd < 0) {
    throw std::runtime_error("create socket failed");
  }
  _recive_thread = std::thread(&Udp::receive_loop, this);
}

slip::Udp::~Udp() {
  _finish = true;
  _recive_thread.join();
  close(_socketfd);
}

int slip::Udp::send(std::string dest_ip, unsigned short dest_port, unsigned short source_port, std::string data) {
  //Datagram to represent the packet
  char datagram[DATAGRAM_MAX_LEN], *payload;
  std::string source_ip = slip::get_local_ip();

  //zero out the packet buffer
  memset (datagram, 0, DATAGRAM_MAX_LEN);

  //UDP header
  struct udphdr *udph = (struct udphdr *) (datagram);

  struct sockaddr_in destaddr, sourceaddr;

  //Address config
  destaddr.sin_family = AF_INET;
  destaddr.sin_port = htons(dest_port); // 目的端口
  destaddr.sin_addr.s_addr = inet_addr(dest_ip.c_str()); // 目的ip

  socklen_t destaddr_len = sizeof(destaddr);

  sourceaddr.sin_family = AF_INET;
  sourceaddr.sin_port = htons(source_port); // 本机端口
  sourceaddr.sin_addr.s_addr = inet_addr(source_ip.c_str()); // 本机ip

  // socklen_t sourceaddr_len = sizeof(sourceaddr);

  //Data part
  unsigned short payload_len = strlen(data.c_str()) + sizeof(struct udphdr);
  // std::cout << payload_len << std::endl;
  payload = datagram + sizeof(struct udphdr);
  strcpy(payload, data.c_str());

  #ifdef __APPLE__ // macOS

  //UDP header
  udph->uh_sport = sourceaddr.sin_port;
  udph->uh_dport = destaddr.sin_port;
  udph->uh_ulen = htons(8 + data.length()); //udp header size
  // calculate checksum
  udph->uh_sum = slip::calc_checksum(sourceaddr.sin_addr.s_addr, destaddr.sin_addr.s_addr, IPPROTO_UDP, payload, payload_len);

  #elif __linux__ // linux

  //UDP header
  udph->source = sourceaddr.sin_port;
  udph->dest = destaddr.sin_port;
  udph->len = htons(8 + data.length()); //udp header size
  // calculate checksum
  std::cout << "send" << std::endl;
  std::cout << (sourceaddr.sin_addr.s_addr) << " " << (destaddr.sin_addr.s_addr) << std::endl;
  std::cout << inet_ntoa(sourceaddr.sin_addr) << " " << inet_ntoa(destaddr.sin_addr) << std::endl;
  udph->check = slip::calc_checksum(sourceaddr.sin_addr.s_addr, destaddr.sin_addr.s_addr, IPPROTO_UDP, payload, payload_len);
  std::cout << "checksum: " << udph->check << std::endl;

  #endif


  return sendto(_socketfd, datagram, payload_len, 0, (struct sockaddr *) &destaddr, destaddr_len);
}

slip::Udp::listener_ptr slip::Udp::add_listener(unsigned short port, slip::Udp::listener func) {
  _table[port].push_front(func);
  return _table[port].begin();
}

bool slip::Udp::remove_listener(unsigned short port, slip::Udp::listener_ptr ptr) {
  _table[port].erase(ptr);
  return true;
}

void slip::Udp::receive_loop() {

  sockaddr_in sourceaddr;

  sourceaddr.sin_family = AF_INET;
  sourceaddr.sin_addr.s_addr = inet_addr(slip::get_local_ip().c_str()); // 本机ip

  socklen_t sourceaddr_len = sizeof(sourceaddr);

  if (bind(_socketfd, (struct sockaddr *) &sourceaddr, sourceaddr_len) < 0) {
      throw std::runtime_error("bind failed");
  }

  int tot_len;
  char datagram[DATAGRAM_MAX_LEN];

  while (!_finish) {
    if ((tot_len = recvfrom (_socketfd, datagram, sizeof(datagram), 0, (struct sockaddr *) &sourceaddr, &sourceaddr_len)) != -1) {

      struct ip *iphd = (struct ip *) (datagram);
      struct udphdr *udph = (struct udphdr *) (datagram + sizeof(struct ip));
      char *data = (char *) (datagram + sizeof(ip) + sizeof(struct udphdr));

      // std::cout << tot_len - sizeof(struct ip) << std::endl;
      // std::cout << udph->check << std::endl;

      #ifdef __APPLE__ // macOS

      unsigned short checksum = udph->uh_sum;
      udph->uh_sum = 0;

      #elif __linux__ // linux

      unsigned short checksum = udph->check;
      udph->uh_sum = 0;

      #endif

      bool verify = slip::verify_checksum(iphd->ip_src.s_addr, iphd->ip_dst.s_addr, IPPROTO_UDP, (char*)udph, tot_len - sizeof(struct ip), checksum);

      std::cout << "receive" << std::endl;
      std::cout << (iphd->ip_src.s_addr) << " " << (iphd->ip_dst.s_addr) << std::endl;
      std::cout << inet_ntoa(iphd->ip_src) << " " << inet_ntoa(iphd->ip_dst) << std::endl;
      std::cout << "receive checksum: " << checksum << std::endl;
      std::cout << "calculate checksum: " << slip::calc_checksum(iphd->ip_src.s_addr, iphd->ip_dst.s_addr, IPPROTO_UDP, (char*)udph, tot_len - sizeof(struct ip)) << std::endl;

      if (verify) {

        std::string source_ip = std::string(inet_ntoa(iphd->ip_src));
        std::string data_str = std::string(data, tot_len - sizeof(struct ip) - sizeof(struct udphdr));

        #ifdef __APPLE__ // macOS

        unsigned short source_port = ntohs(udph->uh_sport);
        unsigned short dest_port = ntohs(udph->uh_dport);

        #elif __linux__ // linux

        unsigned short source_port = ntohs(udph->source);
        unsigned short dest_port = ntohs(udph->dest);

        #endif

        for (auto it = _table[dest_port].begin(); it != _table[dest_port].end(); ++it) {
          (*it)(source_ip, source_port, data_str);
        }

      } else {
        // valid checksum
        std::cout << "valid checksum" << std::endl;
      }
    }
  }

}
