#ifndef __TCP__HPP__
#define __TCP__HPP__ 1

#include <string>
#include <thread>
#include <atomic>
#include <list>
#include <functional>
#include <map>

namespace slip {

class Tcp {
public:

  enum tcp_state {
    //
    // TODO: states
    //
  };

  struct tcp_pcb {
    unsigned int dest_ip;
    unsigned short dest_port, source_port;
    tcp_state state;
    unsigned int last_seq;
  };

  typedef std::function<void(std::string)> listener;

  typedef std::list<listener>::iterator listener_ptr;

  typedef std::map<unsigned short, std::list<listener> > listener_table;

  Tcp();

  ~Tcp();

  tcp_pcb* connect(std::string dest_ip, unsigned short dest_port, unsigned short source_port);

  int send(tcp_pcb* pcb, std::string data);

  listener_ptr add_listener(tcp_pcb* pcb, listener func);

  bool remove_listener(tcp_pcb* pcb, listener_ptr ptr);

  void close(tcp_pcb* pcb);

private:

  struct tcp_flags {
    unsigned int seq;       // 序列号
    unsigned int ack_seq;   // ack 序列号
    bool fin;
    bool syn;
    bool rst;
    bool psh;
    bool ack;
  };

  int send(tcp_pcb* pcb, tcp_flags flags, std::string data);

  void receive_loop();

  int _socketfd;
  std::thread _recive_thread;
  listener_table _table;
  std::atomic<bool> _finish;
};

}

#endif // __TCP__HPP__
