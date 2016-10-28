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

int slip::Tcp::tcp_pcb::send(std::string data) {
  // wait till able to send
  while (state != ESTABLISHED) {}

  slip::Tcp::tcp_flags flags;
  flags.seq = ++last_seq;
  flags.psh = true;

  state = WAIT_ACK;

  return send(flags, data);
}

slip::Tcp::pcb_listener_ptr slip::Tcp::tcp_pcb::add_listener(slip::Tcp::pcb_listener func) {
  listeners.push_front(func);
  return listeners.begin();
}

bool slip::Tcp::tcp_pcb::remove_listener(slip::Tcp::pcb_listener_ptr ptr) {
  listeners.erase(ptr);
  return true;
}

void slip::Tcp::tcp_pcb::close() {
  // wait till able to close
  while (state != ESTABLISHED) {}

  slip::Tcp::tcp_flags flags;
  flags.seq = ++last_seq;
  flags.fin = true;

  state = FIN_SENT;

  send(flags, "");
}

int slip::Tcp::tcp_pcb::send(slip::Tcp::tcp_flags flags, std::string data) {
  // remenber flags and data to resend
  last_flags = flags;
  last_data = data;

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
  tcph->th_seq = htonl(flags.seq);
  tcph->th_ack = htonl(flags.ack_seq);
  tcph->th_off = 5;
  // set flags
  unsigned short th_flags = 0;
  th_flags |= flags.fin ? TH_FIN : 0;
  th_flags |= flags.syn ? TH_SYN : 0;
  th_flags |= flags.rst ? TH_RST : 0;
  th_flags |= flags.psh ? TH_PUSH : 0;
  th_flags |= flags.ack ? TH_ACK : 0;

  tcph->th_flags = th_flags;

  tcph->th_win = htons (65535);    /* maximum allowed window size */
  tcph->th_sum = 0;    //leave checksum 0 now, filled later by pseudo header
  tcph->th_urp = 0;

  // calculate checksum

  tcph->th_sum = slip::calc_checksum(sourceaddr.sin_addr.s_addr, destaddr.sin_addr.s_addr, IPPROTO_TCP, datagram, datagram_len);

  #elif __linux__ // linux

  //TCP Header
  tcph->source = sourceaddr.sin_port;
  tcph->dest = destaddr.sin_port;
  tcph->seq = htonl(flags.seq);
  tcph->ack_seq = htonl(flags.ack_seq);
  tcph->doff = 5;

  tcph->fin = flags.fin;
  tcph->syn = flags.syn;
  tcph->rst = flags.rst;
  tcph->psh = flags.psh;
  tcph->ack = flags.ack;

  tcph->urg = 0;
  tcph->window = htons (65535);    /* maximum allowed window size */
  tcph->check = 0;    //leave checksum 0 now, filled later by pseudo header
  tcph->urg_ptr = 0;

  // calculate checksum

  tcph->check = slip::calc_checksum(sourceaddr.sin_addr.s_addr, destaddr.sin_addr.s_addr, IPPROTO_TCP, datagram, datagram_len);

  #endif

  return sendto(tcp->_socketfd, datagram, datagram_len, 0, (struct sockaddr *) &destaddr, destaddr_len);
}

void slip::Tcp::tcp_pcb::action(slip::Tcp::tcp_flags flags, std::string data) {
  switch (state) {
    case CLOSED:
      // 已关闭连接, 无事件响应
      break;
    case LISTEN:
      // 接收 SYN, 发送 ACK & SYN, 转到 SYN_RCVD
      if (!flags.fin & flags.syn & !flags.rst & !flags.psh & !flags.ack & (flags.seq == last_seq + 1)) {
        flags.ack_seq = flags.seq;
        last_seq = ++flags.seq;
        flags.ack = true;

        state = SYN_RCVD;

        send(flags, "");
      }
      break;
    case SYN_SENT:
      // 接收 ACK & SYN, 发送 ACK, 转到 ESTABLISHED
      if (!flags.fin & flags.syn & !flags.rst & !flags.psh & flags.ack & (flags.ack_seq == last_seq) & (flags.seq == last_seq + 1)) {
        flags.ack_seq = flags.seq;
        flags.seq = ++last_seq;
        flags.syn = false;

        state = ESTABLISHED;

        send(flags, "");
      }
      break;
    case SYN_RCVD:
      // 接收 ACK, 不发送, 转到 ESTABLISHED
      if (!flags.fin & !flags.syn & !flags.rst & !flags.psh & flags.ack & (flags.ack_seq == last_seq) & (flags.seq == last_seq + 1)) {
        state = ESTABLISHED;
      }
      break;
    case ESTABLISHED:
      // 接收 PSH, 发送 ACK, 不转移, 触发 listener
      // 接收 FIN, 发送 FIN & ACK 转到 FIN_RCVD
      if (!flags.fin & !flags.syn & !flags.rst & flags.psh & !flags.ack & (flags.seq == last_seq + 1)) {
        flags.ack_seq = flags.seq;
        flags.seq = ++last_seq;
        flags.psh = false;
        flags.ack = true;

        send(flags, "");

        for (auto it = listeners.begin(); it != listeners.end(); ++it) {
          (*it)(data);
        }
      } else if (flags.fin & !flags.syn & !flags.rst & !flags.psh & !flags.ack & (flags.seq == last_seq + 1)) {
        flags.ack_seq = flags.seq;
        flags.seq = ++last_seq;
        flags.ack = true;

        state = FIN_RCVD;

        send(flags, "");
      }
      break;
    case FIN_SENT:
      // 接受 FIN & ACK, 发送 ACK, 转到 CLOSED
      if (flags.fin & !flags.syn & !flags.rst & !flags.psh & flags.ack & (flags.ack_seq == last_seq) & (flags.seq == last_seq + 1)) {
        flags.ack_seq = flags.seq;
        flags.seq = ++last_seq;
        flags.fin = false;

        state = CLOSED;

        send(flags, "");
      }
      break;
    case FIN_RCVD:
      // 接受 ACK, 不发送, 转到 CLOSED
      if (!flags.fin & !flags.syn & !flags.rst & !flags.psh & flags.ack & (flags.ack_seq == last_seq) & (flags.seq == last_seq + 1)) {
        state = CLOSED;
      }
      break;
    case WAIT_ACK:
      // 接受 ACK, 不发送, 转到 ESTABLISHED
      if (!flags.fin & !flags.syn & !flags.rst & !flags.psh & flags.ack & (flags.ack_seq == last_seq) & (flags.seq == last_seq + 1)) {
        state = ESTABLISHED;
      }
      break;
  }
}

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

slip::Tcp::tcp_pcb_ptr slip::Tcp::connect(std::string dest_ip, unsigned short dest_port, unsigned short source_port) {
  auto pcb = std::make_shared<slip::Tcp::tcp_pcb>();
  pcb->dest_ip = dest_ip;
  pcb->dest_port = dest_port;
  pcb->source_port = source_port;
  pcb->state = SYN_SENT;
  pcb->last_seq = 0;
  pcb->tcp = this;

  slip::Tcp::tcp_flags flags;
  flags.seq = 0;
  flags.syn = true;

  _table[source_port] = pcb;

  pcb->send(flags, "");

  return pcb;
}

slip::Tcp::tcp_pcb_ptr slip::Tcp::listen(unsigned short source_port) {
  auto pcb = std::make_shared<slip::Tcp::tcp_pcb>();
  pcb->source_port = source_port;
  pcb->state = LISTEN;
  pcb->last_seq = 0;
  pcb->tcp = this;

  _table[source_port] = pcb;

  return pcb;
}


void slip::Tcp::receive_loop() {
  sockaddr_in sourceaddr;
  std::string source_ip = slip::get_local_ip();
  // std::string source_ip = "127.0.0.1";

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
      struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof(struct ip));
      char *data = (char *) (datagram + sizeof(ip) + sizeof(struct tcphdr));

      // std::cout << tot_len - sizeof(struct ip) << std::endl;
      // std::cout << tcph->check << std::endl;

      #ifdef __APPLE__ // macOS

      unsigned short checksum = tcph->th_sum;

      #elif __linux__ // linux

      unsigned short checksum = tcph->check;

      #endif

      bool verify = slip::verify_checksum(iphd->ip_src.s_addr, iphd->ip_dst.s_addr, IPPROTO_TCP, (char*)tcph, tot_len - sizeof(struct ip), checksum);

      std::cout << "receive" << std::endl;
      std::cout << (iphd->ip_src.s_addr) << " " << (iphd->ip_dst.s_addr) << std::endl;
      std::cout << inet_ntoa(iphd->ip_src) << " " << inet_ntoa(iphd->ip_dst) << std::endl;
      std::cout << "receive checksum: " << checksum << std::endl;

      if (verify) {

        std::string source_ip = std::string(inet_ntoa(iphd->ip_src));
        std::string data_str = std::string(data, tot_len - sizeof(struct ip) - sizeof(struct tcphdr));
        slip::Tcp::tcp_flags flags;

        #ifdef __APPLE__ // macOS

        unsigned short source_port = ntohs(tcph->th_sport);
        unsigned short dest_port = ntohs(tcph->th_dport);

        // set flags
        flags.seq = ntohl(tcph->th_seq);
        flags.ack_seq = ntohl(tcph->th_ack);
        flags.fin = !!(tcph->th_flags & TH_FIN);
        flags.syn = !!(tcph->th_flags & TH_SYN);
        flags.rst = !!(tcph->th_flags & TH_RST);
        flags.psh = !!(tcph->th_flags & TH_PUSH);
        flags.ack = !!(tcph->th_flags & TH_ACK);

        #elif __linux__ // linux

        unsigned short source_port = ntohs(tcph->source);
        unsigned short dest_port = ntohs(tcph->dest);

        // set flags
        flags.seq = ntohl(tcph->seq);
        flags.ack_seq = ntohl(tcph->ack_seq);
        flags.fin = tcph->fin;
        flags.syn = tcph->syn;
        flags.rst = tcph->rst;
        flags.psh = tcph->psh;
        flags.ack = tcph->ack;

        #endif

        if (_table.find(dest_port) != _table.end()) {
          auto pcb = _table[dest_port];
          if (pcb->state != CLOSED) {
            if (pcb->state == LISTEN) {
              pcb->dest_ip = source_ip;
              pcb->dest_port = source_port;
            }
            pcb->action(flags, data_str);
          } else {
            // already closed
            _table.erase(dest_port);
          }
        }

      } else {
        // valid checksum
        std::cout << "invalid checksum" << std::endl;
      }
    }
  }

}
