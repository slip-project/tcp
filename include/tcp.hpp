#ifndef __TCP__HPP__
#define __TCP__HPP__ 1

#include <string>
#include <thread>
#include <atomic>
#include <list>
#include <functional>
#include <map>
#include <memory>
#include "timeout.hpp"

namespace slip {

/**
 * TCP 类
 */
class Tcp {

private:

  /**
   * TCP的标志位
   */
  struct tcp_flags {
    unsigned int seq;       // 序列号
    unsigned int ack_seq;   // ack 序列号
    bool fin;
    bool syn;
    bool rst;
    bool psh;
    bool ack;
  };

public:

  /**
   * TCP状态表
   */
  enum tcp_state {
    CLOSED      = 0,
    LISTEN      = 1,
    SYN_SENT    = 2,
    SYN_RCVD    = 3,
    ESTABLISHED = 4,
    FIN_SENT    = 5,
    FIN_RCVD    = 6,
    WAIT_ACK    = 7
  };

  /**
   * TCP控制块
   */
  class tcp_pcb;

  /**
   * TCP控制块指针
   */
  typedef std::shared_ptr<tcp_pcb> tcp_pcb_ptr;

  /**
   * TCP控制块监听器
   */
  typedef std::function<void(std::string)> pcb_listener;

  /**
   * TCP控制块监听器指针
   */
  typedef std::list<pcb_listener>::iterator pcb_listener_ptr;

  /**
   * TCP控制块表
   */
  typedef std::map<unsigned short, tcp_pcb_ptr> pcb_table;

  /**
   * TCP控制块
   */
  class tcp_pcb {
    public:
      std::string dest_ip;
      unsigned short dest_port, source_port;
      tcp_state state;
      unsigned int last_seq;
      Timeout::timer_pcb_ptr timer;

      /**
       * [send 发送数据方法]
       * @param  data [要发送的数据]
       * @return      [description]
       */
      int send(std::string data);

      /**
       * [add_listener 控制块监听器添加方法]
       * @param  func [监听函数，lambda表达式]
       * @return      [pcb_listener_ptr, 控制块监听器指针]
       */
      pcb_listener_ptr add_listener(pcb_listener func);

      /**
       * [remove_listener 控制块监听器删除方法]
       * @param  ptr [pcb_listener_ptr , 控制块监听器添加方法返回的指针]
       * @return     [bool , 删除操作是否成功]
       */
      bool remove_listener(pcb_listener_ptr ptr);

      /**
       * [close TCP连接关闭方法]
       */
      void close();
      tcp_pcb() {};
    private:
      std::list<pcb_listener> listeners;

      Tcp* tcp;



      /**
       * [send 发送数据方法]
       * @param  flags [TCP标志位]
       * @param  data  [要发送的数据]
       * @return       [description]
       */
      int send(tcp_flags flags, std::string data);

      /**
       * [action TCP状态机]
       * @param flags [TCP标志位]
       * @param data  [要发送的数据]
       */
      void action(tcp_flags flags, std::string data);

    friend Tcp;
  };

  Tcp();
  ~Tcp();

  /**
   * [connect 建立TCP连接的方法]
   * @param  dest_ip     [目的主机IP]
   * @param  dest_port   [目的主机端口]
   * @param  source_port [源主机端口]
   * @return             [tcp_pcb_ptr , TCP控制块指针]
   */
  tcp_pcb_ptr connect(std::string dest_ip, unsigned short dest_port, unsigned short source_port);

  /**
   * [listen TCP端口监听方法]
   * @param  source_port [监听源端口]
   * @return             [tcp_pcb_ptr , TCP控制块指针]
   */
  tcp_pcb_ptr listen(unsigned short source_port);

private:

  /**
   * [receive_loop 监听循环]
   */
  void receive_loop();

  int _socketfd;
  std::thread _recive_thread;
  pcb_table _table;
  std::atomic<bool> _finish;
  Timeout _timeout;
};

}

#endif // __TCP__HPP__
