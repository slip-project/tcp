#include "utils.hpp"
#include <cstring>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>


  /**
   * [csum 校验和计算方法]
   * @param  ptr    [存放原始数据的short数组]
   * @param  nbytes [需要计算的字节长度]
   * @return        [返回一个无符号短整型数 为校验和]
   */
unsigned short slip::csum(unsigned short *ptr, int nbytes) {
  long sum;
  unsigned short oddbyte;
  unsigned short answer;

  sum = 0;
  while (nbytes > 1) {
      sum += *ptr++;
      nbytes -= 2;
  }
  if (nbytes == 1) {
      oddbyte = 0;
      *((u_char*) &oddbyte) = *(u_char*) ptr;
      sum += oddbyte;
  }

  sum = (sum >> 16) + (sum & 0xffff);
  sum = sum + (sum >> 16);
  answer = (short) ~sum;

  return answer;
}


 /**
   * [calc_checksum 校验和计算方法]
   * @param  source_ip   [源主机ip地址]
   * @param  dest_ip     [目的主机ip地址]
   * @param  protocol    [8位协议号]
   * @param  payload     [数据包]
   * @param  payload_len [数据包长度]
   * @return             [无符号短整型数，为校验和]
   */
unsigned short slip::calc_checksum(unsigned long source_ip, unsigned long dest_ip, u_int8_t protocol,
                             char* payload, unsigned short payload_len) {

  pseudo_header psh;

  psh.source_address = source_ip;
  psh.dest_address = dest_ip;
  psh.placeholder = 0;
  psh.protocol = protocol;
  psh.udp_length = htons(payload_len);

  int psize = sizeof(struct pseudo_header) + payload_len;
  char *pseudogram = new char[psize];

  memcpy(pseudogram, (char*) &psh, sizeof(struct pseudo_header));
  memcpy(pseudogram + sizeof(struct pseudo_header), payload, payload_len);

  unsigned short sum = csum((unsigned short*) pseudogram, psize);

  delete [] pseudogram;
  return sum;
}

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
bool slip::verify_checksum(unsigned long source_ip, unsigned long dest_ip, u_int8_t protocol,
                     char* payload, unsigned short payload_len, unsigned short checksum) {
  unsigned short sum = calc_checksum(source_ip, dest_ip, protocol, payload, payload_len);
  return sum == 0;

}


std::string slip::local_ip = "";



  /**
   * [get_local_ip 获取本地IP的方法]
   * @return [本地IP]
   */
std::string slip::get_local_ip() {

  if (local_ip != "") return local_ip;

  struct ifaddrs * ifAddrStruct=NULL;
  struct ifaddrs * ifa=NULL;
  void * tmpAddrPtr=NULL;

  getifaddrs(&ifAddrStruct);

  for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {

    #ifdef __APPLE__ // macOS

    if (std::string(ifa->ifa_name) == "en0" && ifa->ifa_addr) {

    #elif __linux__ //linux

    if (std::string(ifa->ifa_name) == "eth0" && ifa->ifa_addr) {

    #endif

      if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
          // is a valid IP4 Address
          tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
          char addressBuffer[INET_ADDRSTRLEN];
          inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
          if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
          local_ip = std::string(addressBuffer);
          break;

      } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
          // is a valid IP6 Address
          tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
          char addressBuffer[INET6_ADDRSTRLEN];
          inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
          if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
          local_ip = std::string(addressBuffer);
          break;
      }
    }
  }
  return local_ip;
}

