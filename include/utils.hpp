#ifndef __UTILS__HPP__
#define __UTILS__HPP__ 1

#include <string>
#include <cstdlib>

namespace slip {

  struct pseudo_header {
    u_int32_t source_address;
    u_int32_t dest_address;
    u_int8_t placeholder;
    u_int8_t protocol;
    u_int16_t udp_length;
  };

  unsigned short csum(unsigned short *ptr, int nbytes);

  unsigned short calc_checksum(unsigned long source_ip, unsigned long dest_ip, u_int8_t protocol,
                               char* payload, unsigned short payload_len);

  bool verify_checksum(unsigned long source_ip, unsigned long dest_ip, u_int8_t protocol,
                       char* payload, unsigned short payload_len, unsigned short checksum);

  extern std::string local_ip;

  std::string get_local_ip();

}

#endif // __UTILS__HPP__
