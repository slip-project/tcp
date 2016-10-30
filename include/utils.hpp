#ifndef __UTILS__HPP__
#define __UTILS__HPP__ 1

#include <string>
#include <cstdlib>

namespace slip {

  /**
   * TCP 伪头部， 在计算校验和的过程中需要使用到。
   * 使用原因请参考 http://stackoverflow.com/questions/359045/what-is-the-significance-of-pseudo-header-used-in-udp-tcp
   * 实现原理请参考 http://baike.baidu.com/link?url=xosNMBLUMckvWEmrLX0-Srv16wms5AAiFzkvXWMWGB7MPAN12bbOEUTRtL33TT0L9o4TB--F8sLaL8SFiD_ATK
   */
  struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
  };

  /**
   * [csum 校验和计算方法]
   * @param  ptr    [存放原始数据的short数组]
   * @param  nbytes [需要计算的字节长度]
   * @return        [返回一个无符号短整型数 为校验和]
   */
  unsigned short csum(unsigned short *ptr, int nbytes);


  /**
   * [calc_checksum 校验和计算方法]
   * @param  source_ip   [源主机ip地址]
   * @param  dest_ip     [目的主机ip地址]
   * @param  protocol    [8位协议号]
   * @param  payload     [数据包]
   * @param  payload_len [数据包长度]
   * @return             [无符号短整型数，为校验和]
   */
  unsigned short calc_checksum(unsigned long source_ip, unsigned long dest_ip, u_int8_t protocol,
                               char* payload, unsigned short payload_len);


  /**
   * [verify_checksum 校验和检查方法]
   * @param  source_ip   [源主机ip地址]
   * @param  dest_ip     [目的主机ip地址]
   * @param  protocol    [8位协议号]
   * @param  payload     [数据包]
   * @param  payload_len [数据包长度]
   * @param  checksum    [传入的初始校验和]
   * @return             [布尔值，true则表示校验和合法]
   */
  bool verify_checksum(unsigned long source_ip, unsigned long dest_ip, u_int8_t protocol,
                       char* payload, unsigned short payload_len, unsigned short checksum);

  /**
   * 本地IP
   */
  extern std::string local_ip;

  /**
   * [get_local_ip 获取本地IP的方法]
   * @return [本地IP]
   */
  std::string get_local_ip();

}

#endif // __UTILS__HPP__
