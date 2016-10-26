#ifndef __UDP__HPP__
#define __UDP__HPP__ 1

#include <string>
#include <thread>
#include <atomic>
#include <list>
#include <functional>
#include <map>

namespace slip {

class Udp {
public:

  typedef std::function<void(std::string, unsigned short, std::string)> listener;

  typedef std::list<listener>::const_iterator listener_ptr;

  typedef std::map<unsigned short, std::list<listener>> listener_table;

  Udp();

  ~Udp();

  int send(std::string dest_ip, unsigned short dest_port, unsigned short source_port, std::string data);

  listener_ptr add_listener(unsigned short port, Udp::listener func);

  bool remove_listener(unsigned short port, Udp::listener_ptr ptr);

private:

  void receive_loop();

  int _socketfd;
  std::thread _recive_thread;
  listener_table _table;
  std::atomic<bool> _finish;
};

}

#endif // __UDP__HPP__
