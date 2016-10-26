#ifndef __IP__HPP__
#define __IP__HPP__ 1

#include <string>

class Ip {
public:
  struct tcp_header {
    unsigned short source;  // 发送端口
    unsigned short dest;    // 目的端口
    unsigned int seq;       // 序列号
    unsigned int ack_seq;   // ack 序列号
    unsigned short doff;    // tcp 头部大小
    bool fin;
    bool syn;
    bool rst;
    bool psh;
    bool ack;
    unsigned short window;  // 最大窗口大小
    unsigned short check;   // 检验和
    unsigned short urg_ptr; // 紧急指针
  };
  struct udp_header {
    unsigned short source;  // 发送端口
    unsigned short dest;    // 目的端口
    unsigned short len;     // 头部长度?数据长度?
    unsigned short check;   // 检验和
  };
  struct header {

  };

};

#endif // __IP__HPP__
