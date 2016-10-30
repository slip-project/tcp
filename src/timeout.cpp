#include "timeout.hpp"

#define DEBUG

#ifdef DEBUG

#include <iostream>

#endif

slip::Timeout::Timeout() {
  _finish = false;
  _timer_thread = std::thread(&Timeout::timer_loop, this);
}

slip::Timeout::~Timeout() {
  _finish = true;
  _timer_thread.join();
}

slip::Timeout::timer_pcb_ptr slip::Timeout::add_timer(slip::Timeout::time_interval timeout, slip::Timeout::timer_callback callback) {
  auto pcb = std::make_shared<timer_pcb>();
  pcb->target_epoch = now() + timeout;
  pcb->callback = callback;
  pcb->enable = true;

  _timer_heap.push(pcb);

  return pcb;
}

slip::Timeout::time_interval slip::Timeout::now() {
  return std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

void slip::Timeout::timer_loop() {
  while (!_finish) {
    auto now_time = now();
    if (!_timer_heap.empty() && _timer_heap.top()->target_epoch <= now_time) {

      auto pcb = _timer_heap.top();
      _timer_heap.pop();

      #ifdef DEBUG

      std::cout << "===== timer triggered =====" << std::endl;
      std::cout << "target: " << pcb->target_epoch << std::endl;
      std::cout << "now: " << now_time << std::endl;
      std::cout << "enable: " << (pcb->enable ? "true" : "false") << std::endl;
      std::cout << "===========================" << std::endl;

      #endif

      if (pcb->enable) {
        pcb->callback();
      }

    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}
