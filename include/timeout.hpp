#ifndef __TIMEOUT__HPP__
#define __TIMEOUT__HPP__ 1

#include <chrono>
#include <thread>
#include <functional>
#include <atomic>
#include <vector>
#include <queue>
#include <memory>

namespace slip {

class Timeout {

public:
  typedef std::function<void(void)> timer_callback;

  typedef std::chrono::milliseconds::rep time_interval;

  struct timer_pcb {
    time_interval target_epoch;
    timer_callback callback;
    std::atomic<bool> enable;
  };

  typedef std::shared_ptr<timer_pcb> timer_pcb_ptr;

  Timeout();

  ~Timeout();

  timer_pcb_ptr add_timer(time_interval timeout, timer_callback callback);

  class compare_pcb_ptr {
  public:
    bool operator() (const timer_pcb_ptr & a, const timer_pcb_ptr & b) {
      return a->target_epoch > b->target_epoch;
    }
  };

private:

  static inline time_interval now();

  void timer_loop();

  std::thread _timer_thread;
  std::priority_queue<timer_pcb_ptr, std::vector<timer_pcb_ptr>, compare_pcb_ptr> _timer_heap;
  std::atomic<bool> _finish;

};

}

#endif // __TIMEOUT__HPP__
