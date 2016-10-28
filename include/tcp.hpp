#ifndef __TCP__HPP__
#define __TCP__HPP__ 1

#include <string>
#include <thread>
#include <atomic>
#include <list>
#include <functional>
#include <map>
#include <memory>

namespace slip {

class Tcp {
public:

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

  struct tcp_pcb;

  typedef std::shared_ptr<tcp_pcb> tcp_pcb_ptr;

  typedef std::function<void(std::string)> pcb_listener;

  typedef std::list<pcb_listener>::iterator pcb_listener_ptr;

  typedef std::map<unsigned short, tcp_pcb_ptr> pcb_table;

  struct tcp_flags {
    unsigned int seq;       // 序列号
    unsigned int ack_seq;   // ack 序列号
    bool fin;
    bool syn;
    bool rst;
    bool psh;
    bool ack;
  };

  struct tcp_pcb {
    std::string dest_ip;
    unsigned short dest_port, source_port;
    tcp_state state;
    unsigned int last_seq;
    std::list<pcb_listener> listeners;
    unsigned int timer;

    tcp_flags last_flags;
    std::string last_data;

    Tcp* tcp;

    int send(std::string data);

    pcb_listener_ptr add_listener(pcb_listener func);

    bool remove_listener(pcb_listener_ptr ptr);

    void close();

    int send(tcp_flags flags, std::string data);

    void action(tcp_flags flags, std::string data);
  };

  Tcp();

  ~Tcp();

  tcp_pcb_ptr connect(std::string dest_ip, unsigned short dest_port, unsigned short source_port);

  tcp_pcb_ptr listen(unsigned short source_port);

private:


  void receive_loop();

  int _socketfd;
  std::thread _recive_thread;
  pcb_table _table;
  std::atomic<bool> _finish;
};

}

#endif // __TCP__HPP__
