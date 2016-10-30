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

#define DEBUG
#define DATAGRAM_MAX_LEN 4096

#ifdef DEBUG

#include <iostream>

#endif

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


  /**
   * [send 数据报文包发送方法]
   * @param  dest_ip     [目的主机ip地址]
   * @param  dest_port   [目的主机端口]
   * @param  source_port [源主机端口]
   * @param  data        [发送的数据，字符串形式]
   * @return             [调用系统默认的sendto函数的返回值]
   */
int slip::Udp::send(std::string dest_ip, unsigned short dest_port, unsigned short source_port, std::string data) {

  #ifdef DEBUG

  std::cout << "===== udp send datagram =====" << std::endl;
  std::cout << "remote client: " << dest_ip << ":" << dest_port << std::endl;
  std::cout << "local port: " << source_port << std::endl;
  std::cout << "data: " << data << std::endl;
  std::cout << "=============================" << std::endl;

  #endif

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


  //Data part
  unsigned short datagram_len = data.length() + sizeof(struct udphdr);

  payload = datagram + sizeof(struct udphdr);
  strcpy(payload, data.c_str());



  #ifdef __APPLE__ // macOS

  //UDP header
  udph->uh_sport = sourceaddr.sin_port;
  udph->uh_dport = destaddr.sin_port;
  udph->uh_ulen = htons(8 + data.length()); //udp header size
  // calculate checksum
  udph->uh_sum = slip::calc_checksum(sourceaddr.sin_addr.s_addr, destaddr.sin_addr.s_addr, IPPROTO_UDP, datagram, datagram_len);
  // std::cout << "checksum: " << udph->uh_sum << std::endl;

  #elif __linux__ // linux

  //UDP header
  udph->source = sourceaddr.sin_port;
  udph->dest = destaddr.sin_port;
  udph->len = htons(8 + data.length()); //udp header size
  // calculate checksum
  udph->check = slip::calc_checksum(sourceaddr.sin_addr.s_addr, destaddr.sin_addr.s_addr, IPPROTO_UDP, datagram, datagram_len);
  // std::cout << "checksum: " << udph->check << std::endl;

  #endif


  return sendto(_socketfd, datagram, datagram_len, 0, (struct sockaddr *) &destaddr, destaddr_len);
}


  /**
   * [add_listener 添加监听器方法]
   * @param  port [监听端口]
   * @param  func [监听函数，测试中用的是lambda形式]
   * @return      [listener_ptr , 监听器的指针]
   */
slip::Udp::listener_ptr slip::Udp::add_listener(unsigned short port, slip::Udp::listener func) {
  _table[port].push_front(func);
  return _table[port].begin();
}

  /**
   * [remove_listener 移除监听器方法]
   * @param  port [监听端口]
   * @param  ptr  [监听器的指针]
   * @return      [布尔值，移除操作处理结果]
   */
bool slip::Udp::remove_listener(unsigned short port, slip::Udp::listener_ptr ptr) {
  _table[port].erase(ptr);
  return true;
}


void slip::Udp::receive_loop() {

  sockaddr_in sourceaddr;
  std::string source_ip = slip::get_local_ip();

  sourceaddr.sin_family = AF_INET;
  sourceaddr.sin_addr.s_addr = inet_addr(source_ip.c_str()); // 本机ip

  socklen_t sourceaddr_len = sizeof(sourceaddr);

  if (bind(_socketfd, (struct sockaddr *) &sourceaddr, sourceaddr_len) < 0) {
      throw std::runtime_error("bind failed");
  }

  int tot_len;
  char datagram[DATAGRAM_MAX_LEN];

  while (!_finish) {
    if ((tot_len = recvfrom (_socketfd, datagram, sizeof(datagram), MSG_DONTWAIT, (struct sockaddr *) &sourceaddr, &sourceaddr_len)) != -1) {

      struct ip *iphd = (struct ip *) (datagram);
      struct udphdr *udph = (struct udphdr *) (datagram + sizeof(struct ip));
      char *data = (char *) (datagram + sizeof(ip) + sizeof(struct udphdr));

      #ifdef __APPLE__ // macOS

      unsigned short checksum = udph->uh_sum;

      #elif __linux__ // linux

      unsigned short checksum = udph->check;

      #endif

      bool verify = slip::verify_checksum(iphd->ip_src.s_addr, iphd->ip_dst.s_addr, IPPROTO_UDP, (char*)udph, tot_len - sizeof(struct ip), checksum);

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

        #ifdef DEBUG

        std::cout << "===== udp receive datagram =====" << std::endl;
        std::cout << "remote client: " << source_ip << ":" << source_port << std::endl;
        std::cout << "local port: " << dest_port << std::endl;
        std::cout << "data: " << data_str << std::endl;
        std::cout << "================================" << std::endl;

        #endif

        if (_table.find(dest_port) != _table.end()) {
          for (auto it = _table[dest_port].begin(); it != _table[dest_port].end(); ++it) {
            (*it)(source_ip, source_port, data_str);
          }
        }

      #ifdef DEBUG

      } else {
        std::cout << "invalid checksum" << std::endl;

      #endif

      }
    }
  }

}
