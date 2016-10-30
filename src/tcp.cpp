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

#define DEBUG
#define DATAGRAM_MAX_LEN 4096

#ifdef DEBUG

#include <iostream>

std::string getState(slip::Tcp::tcp_state state) {
  switch (state) {
    case slip::Tcp::CLOSED:
      return "CLOSED";
    case slip::Tcp::LISTEN:
      return "LISTEN";
    case slip::Tcp::SYN_SENT:
      return "SYN_SENT";
    case slip::Tcp::SYN_RCVD:
      return "SYN_RCVD";
    case slip::Tcp::ESTABLISHED:
      return "ESTABLISHED";
    case slip::Tcp::FIN_SENT:
      return "FIN_SENT";
    case slip::Tcp::FIN_RCVD:
      return "FIN_RCVD";
    case slip::Tcp::WAIT_ACK:
      return "WAIT_ACK";
  }
  return "";
}

#endif

/**
 * [send 发送数据方法]
 * @param  data [要发送的数据]
 * @return      [description]
 */
int slip::Tcp::tcp_pcb::send(std::string data) {
  // wait till able to send
  while (state != ESTABLISHED) {}

  slip::Tcp::tcp_flags flags;
  flags.seq = ++last_seq;
  flags.fin = false;
  flags.syn = false;
  flags.rst = false;
  flags.psh = true;
  flags.ack = false;

  state = WAIT_ACK;
  return send(flags, data);
}

/**
 * [add_listener 控制块监听器添加方法]
 * @param  func [监听函数，lambda表达式]
 * @return      [pcb_listener_ptr, 控制块监听器指针]
 */
slip::Tcp::pcb_listener_ptr slip::Tcp::tcp_pcb::add_listener(slip::Tcp::pcb_listener func) {
  listeners.push_front(func);
  return listeners.begin();
}


/**
 * [remove_listener 控制块监听器删除方法]
 * @param  ptr [pcb_listener_ptr , 控制块监听器添加方法返回的指针]
 * @return     [bool , 删除操作是否成功]
 */
bool slip::Tcp::tcp_pcb::remove_listener(slip::Tcp::pcb_listener_ptr ptr) {
  listeners.erase(ptr);
  return true;
}


/**
 * [close TCP连接关闭方法]
 */
void slip::Tcp::tcp_pcb::close() {
  // wait till able to close
  while (state != ESTABLISHED) {}

  slip::Tcp::tcp_flags flags;
  flags.seq = ++last_seq;
  flags.fin = true;
  flags.syn = false;
  flags.rst = false;
  flags.psh = false;
  flags.ack = false;

  state = FIN_SENT;

  send(flags, "");
}


/**
 * [send 发送数据方法]
 * @param  flags [TCP标志位]
 * @param  data  [要发送的数据]
 * @return       [description]
 */
int slip::Tcp::tcp_pcb::send(slip::Tcp::tcp_flags flags, std::string data) {

  #ifdef DEBUG

  std::cout << "===== tcp send datagram =====" << std::endl;
  std::cout << "dest client: " << dest_ip << ":" << dest_port << std::endl;
  std::cout << "local port: " << source_port << std::endl;
  std::cout << "current state: " << getState(state) << std::endl;
  std::cout << "seq: " << flags.seq << std::endl;
  std::cout << "ack seq: " << flags.ack_seq << std::endl;
  std::cout << "flags:" << (flags.fin ? " FIN" : "")
                        << (flags.syn ? " SYN" : "")
                        << (flags.rst ? " RST" : "")
                        << (flags.psh ? " PSH" : "")
                        << (flags.ack ? " ACK" : "")
                        << std::endl;
  std::cout << "data: " << data << std::endl;
  std::cout << "=============================" << std::endl;

  #endif

  // remenber flags and data to resend
  last_flags = flags;
  last_data = data;

  //Datagram to represent the packet
  char datagram[DATAGRAM_MAX_LEN], *payload;
  std::string source_ip = slip::get_local_ip();

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


  //Data part
  unsigned short datagram_len = data.length() + sizeof(struct tcphdr);
  payload = datagram + sizeof(struct tcphdr);
  strcpy(payload, data.c_str());

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


/**
 * [action TCP状态机]
 * @param flags [TCP标志位]
 * @param data  [要发送的数据]
 */
void slip::Tcp::tcp_pcb::action(slip::Tcp::tcp_flags flags, std::string data) {

  #ifdef DEBUG

  std::cout << "===== tcp receive datagram =====" << std::endl;
  std::cout << "remote client: " << dest_ip << ":" << dest_port << std::endl;
  std::cout << "local port: " << source_port << std::endl;
  std::cout << "current state: " << getState(state) << std::endl;
  std::cout << "seq: " << flags.seq << std::endl;
  std::cout << "ack seq: " << flags.ack_seq << std::endl;
  std::cout << "flags:" << (flags.fin ? " FIN" : "")
                        << (flags.syn ? " SYN" : "")
                        << (flags.rst ? " RST" : "")
                        << (flags.psh ? " PSH" : "")
                        << (flags.ack ? " ACK" : "")
                        << std::endl;
  std::cout << "data: " << data << std::endl;

  #endif

  switch (state) {
    case CLOSED:
      // 已关闭连接, 无事件响应
      break;
    case LISTEN:
      // 接收 SYN, 发送 ACK & SYN, 转到 SYN_RCVD
      if (!flags.fin & flags.syn & !flags.rst & !flags.psh & !flags.ack & (flags.seq == last_seq)) {
        flags.ack_seq = flags.seq;
        last_seq = ++flags.seq;
        flags.ack = true;

        state = SYN_RCVD;

        #ifdef DEBUG

        std::cout << "goto state: " << getState(state) << std::endl;
        std::cout << "================================" << std::endl;

        #endif

        send(flags, "");

      #ifdef DEBUG

      } else {
        std::cout << "[DATAGRAM IGNORED]" << std::endl;
        std::cout << "================================" << std::endl;

      #endif

      }
      break;
    case SYN_SENT:
      // 接收 ACK & SYN, 发送 ACK, 转到 ESTABLISHED
      if (!flags.fin & flags.syn & !flags.rst & !flags.psh & flags.ack & (flags.ack_seq == last_seq) & (flags.seq == last_seq + 1)) {
        flags.ack_seq = flags.seq;
        last_seq = ++flags.seq;
        flags.syn = false;

        state = ESTABLISHED;

        #ifdef DEBUG

        std::cout << "goto state: " << getState(state) << std::endl;
        std::cout << "================================" << std::endl;

        #endif

        send(flags, "");

      #ifdef DEBUG

      } else {
        std::cout << "[DATAGRAM IGNORED]" << std::endl;
        std::cout << "================================" << std::endl;

      #endif

      }
      break;
    case SYN_RCVD:
      // 接收 ACK, 不发送, 转到 ESTABLISHED
      if (!flags.fin & !flags.syn & !flags.rst & !flags.psh & flags.ack & (flags.ack_seq == last_seq) & (flags.seq == last_seq + 1)) {
        last_seq = flags.seq;
        state = ESTABLISHED;

        #ifdef DEBUG

        std::cout << "goto state: " << getState(state) << std::endl;
        std::cout << "================================" << std::endl;

      } else {
        std::cout << "[DATAGRAM IGNORED]" << std::endl;
        std::cout << "================================" << std::endl;

      #endif

      }
      break;
    case ESTABLISHED:
      // 接收 PSH, 发送 ACK, 不转移, 触发 listener
      // 接收 FIN, 发送 FIN & ACK 转到 FIN_RCVD
      if (!flags.fin & !flags.syn & !flags.rst & flags.psh & !flags.ack & (flags.seq == last_seq + 1)) {
        flags.ack_seq = flags.seq;
        last_seq = ++flags.seq;
        flags.psh = false;
        flags.ack = true;

        #ifdef DEBUG

        std::cout << "goto state: " << getState(state) << std::endl;
        std::cout << "================================" << std::endl;

        #endif

        send(flags, "");
        for (auto it = listeners.begin(); it != listeners.end(); ++it) {
          (*it)(data);
        }
      } else if (flags.fin & !flags.syn & !flags.rst & !flags.psh & !flags.ack & (flags.seq == last_seq + 1)) {
        flags.ack_seq = flags.seq;
        last_seq = ++flags.seq;
        flags.ack = true;

        state = FIN_RCVD;

        #ifdef DEBUG

        std::cout << "goto state: " << getState(state) << std::endl;
        std::cout << "================================" << std::endl;

        #endif

        send(flags, "");

      #ifdef DEBUG

      } else {
        std::cout << "[DATAGRAM IGNORED]" << std::endl;
        std::cout << "================================" << std::endl;

      #endif

      }
      break;
    case FIN_SENT:
      // 接受 FIN & ACK, 发送 ACK, 转到 CLOSED
      if (flags.fin & !flags.syn & !flags.rst & !flags.psh & flags.ack & (flags.ack_seq == last_seq) & (flags.seq == last_seq + 1)) {
        flags.ack_seq = flags.seq;
        last_seq = ++flags.seq;
        flags.fin = false;

        state = CLOSED;

        #ifdef DEBUG

        std::cout << "goto state: " << getState(state) << std::endl;
        std::cout << "================================" << std::endl;

        #endif

        send(flags, "");

      #ifdef DEBUG

      } else {
        std::cout << "[DATAGRAM IGNORED]" << std::endl;
        std::cout << "================================" << std::endl;

      #endif

      }
      break;
    case FIN_RCVD:
      // 接受 ACK, 不发送, 转到 CLOSED
      if (!flags.fin & !flags.syn & !flags.rst & !flags.psh & flags.ack & (flags.ack_seq == last_seq) & (flags.seq == last_seq + 1)) {
        last_seq = flags.seq;
        state = CLOSED;

        #ifdef DEBUG

        std::cout << "goto state: " << getState(state) << std::endl;
        std::cout << "================================" << std::endl;

      } else {
        std::cout << "[DATAGRAM IGNORED]" << std::endl;
        std::cout << "================================" << std::endl;

      #endif

      }
      break;
    case WAIT_ACK:
      // 接受 ACK, 不发送, 转到 ESTABLISHED
      if (!flags.fin & !flags.syn & !flags.rst & !flags.psh & flags.ack & (flags.ack_seq == last_seq) & (flags.seq == last_seq + 1)) {
        last_seq = flags.seq;
        state = ESTABLISHED;

        #ifdef DEBUG

        std::cout << "goto state: " << getState(state) << std::endl;
        std::cout << "================================" << std::endl;

      } else {
        std::cout << "[DATAGRAM IGNORED]" << std::endl;
        std::cout << "================================" << std::endl;

      #endif

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


/**
 * [connect 建立TCP连接的方法]
 * @param  dest_ip     [目的主机IP]
 * @param  dest_port   [目的主机端口]
 * @param  source_port [源主机端口]
 * @return             [tcp_pcb_ptr , TCP控制块指针]
 */
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
  flags.fin = false;
  flags.syn = true;
  flags.rst = false;
  flags.psh = false;
  flags.ack = false;

  _table[source_port] = pcb;

  pcb->send(flags, "");

  return pcb;
}


/**
 * [listen TCP端口监听方法]
 * @param  source_port [监听源端口]
 * @return             [tcp_pcb_ptr , TCP控制块指针]
 */
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

      #ifdef __APPLE__ // macOS

      unsigned short checksum = tcph->th_sum;

      #elif __linux__ // linux

      unsigned short checksum = tcph->check;

      #endif

      bool verify = slip::verify_checksum(iphd->ip_src.s_addr, iphd->ip_dst.s_addr, IPPROTO_TCP, (char*)tcph, tot_len - sizeof(struct ip), checksum);

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

      #ifdef DEBUG

      } else {
        std::cout << "invalid checksum" << std::endl;

      #endif

      }
    }
  }

}
