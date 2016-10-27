#include "utils.hpp"
#include <cstring>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>

unsigned short slip::csum(unsigned short *ptr, int nbytes) {
  long sum;
  unsigned short oddbyte;
  short answer;

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

// #include <iostream>

// void print(void *buf, int length) {
//     char *bp = (char *) buf;
//     for (int i = 0; i < length; ++i) {
//       int val = bp[i];
//       std::cout << std::hex << val;
//     }
//     std::cout << std::dec << std::endl;
// }

unsigned short slip::calc_checksum(unsigned long source_ip, unsigned long dest_ip, u_int8_t protocol,
                             char* payload, unsigned short payload_len) {

  // std::cout << source_ip << std::endl;
  // std::cout << dest_ip << std::endl;
  // std::cout << protocol << std::endl;
  // print(payload, payload_len);
  // std::cout << payload_len << std::endl;

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

bool slip::verify_checksum(unsigned long source_ip, unsigned long dest_ip, u_int8_t protocol,
                     char* payload, unsigned short payload_len, unsigned short checksum) {

  unsigned short sum = calc_checksum(source_ip, dest_ip, protocol, payload, payload_len);

  // std::cout << "verify_checksum: " << sum << std::endl;

  return sum == 0;

}

std::string slip::local_ip = "";

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

