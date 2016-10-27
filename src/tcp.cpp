#include "tcp.hpp"
#include "utils.hpp"
#include <cctype>
#include <cstring> //memset
#include <stdexcept>
#include <sys/socket.h>  //for socket ofcourse
#include <unistd.h>
#include <stdlib.h> //for exit(0);
#include <errno.h> //For errno - the error number
#include <netinet/tcp.h> //Provides declarations for tcp header
#include <netinet/ip.h>  //Provides declarations for ip header
#include <netinet/in.h>
#include <arpa/inet.h>

#define DATAGRAM_MAX_LEN 4096

#include <iostream>

slip::Tcp::Tcp() {
  _finish = false;
  _socketfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (_socketfd < 0) {
    throw std::runtime_error("create socket failed");
  }
  _recive_thread = std::thread(&Tcp::receive_loop, this);
}

slip::Tcp::~Tcp() {
  _finish = true;
  _recive_thread.join();
  close(_socketfd);
}

slip::Tcp::tcp_pcb* slip::Tcp::connect(std::string dest_ip, unsigned short dest_port, unsigned short source_port);

int slip::Tcp::send(slip::Tcp::tcp_pcb* pcb, std::string data);

int slip::Tcp::send(slip::Tcp::tcp_pcb* pcb, slip::Tcp::tcp_flags flags, std::string data) {
  //Datagram to represent the packet
  char datagram[DATAGRAM_MAX_LEN], *payload;
  std::string source_ip = slip::get_local_ip();
  // std::string source_ip = "127.0.0.1";

  //zero out the packet buffer
  memset (datagram, 0, DATAGRAM_MAX_LEN);

  //UDP header
  struct tcphdr *tcph = (struct tcphdr *) (datagram);

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
  unsigned short datagram_len = data.length() + sizeof(struct tcphdr);
  // std::cout << datagram_len << std::endl;
  payload = datagram + sizeof(struct tcphdr);
  strcpy(payload, data.c_str());

  // std::cout << "send" << std::endl;
  // std::cout << "source ip: " << inet_ntoa(sourceaddr.sin_addr) << std::endl;
  // std::cout << "dest ip: " << inet_ntoa(destaddr.sin_addr) << std::endl;

  #ifdef __APPLE__ // macOS

  // TCP Header
  tcph->th_sport = sourceaddr.sin_port;
  tcph->th_dport = destaddr.sin_port;
  tcph->th_seq = htonl(flags->seq);
  tcph->th_ack = htonl(flags->ack_seq);
  tcph->th_off = 5;
  // TODO set flags

  #elif __linux__ // linux

  // TODO: linux

  #endif


  return sendto(_socketfd, datagram, datagram_len, 0, (struct sockaddr *) &destaddr, destaddr_len);
}

slip::Tcp::listener_ptr slip::Tcp::add_listener(slip::Tcp::tcp_pcb* pcb, slip::Tcp::listener func);

bool slip::Tcp::remove_listener(slip::Tcp::tcp_pcb* pcb, slip::Tcp::listener_ptr ptr);

void slip::Tcp::close(slip::Tcp::tcp_pcb* pcb);

void slip::Tcp::receive_loop();
