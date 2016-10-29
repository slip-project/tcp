#ifndef __UDP__HPP__
#define __UDP__HPP__ 1

#include <string>
#include <thread>
#include <atomic>
#include <list>
#include <functional>
#include <map>

namespace slip {

/**
 * UDP 类
 */
class Udp {

public:

  /**
   * 定义监听器
   */
  typedef std::function<void(std::string, unsigned short, std::string)> listener;

  /**
   * 定义监听器指针
   */
  typedef std::list<listener>::iterator listener_ptr;

  /**
   * 定义监听器表
   */
  typedef std::map<unsigned short, std::list<listener> > listener_table;


  Udp();
  ~Udp();

  /**
   * [send 数据报文包发送方法]
   * @param  dest_ip     [目的主机ip地址]
   * @param  dest_port   [目的主机端口]
   * @param  source_port [源主机端口]
   * @param  data        [发送的数据，字符串形式]
   * @return             [调用系统默认的sendto函数的返回值]
   */
  int send(std::string dest_ip, unsigned short dest_port, unsigned short source_port, std::string data);

  /**
   * [add_listener 添加监听器方法]
   * @param  port [监听端口]
   * @param  func [监听函数，测试中用的是lambda形式]
   * @return      [listener_ptr , 监听器的指针]
   */
  listener_ptr add_listener(unsigned short port, Udp::listener func);

  /**
   * [remove_listener 移除监听器方法]
   * @param  port [监听端口]
   * @param  ptr  [监听器的指针]
   * @return      [布尔值，移除操作处理结果]
   */
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
