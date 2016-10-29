#include "udp.hpp"
#include "tcp.hpp"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <map>
#include <queue>
#define DEBUG
typedef std::queue<std::string> MessageList;
typedef std::map<std::string, MessageList>  MessagesMap;
typedef std::map<std::string, unsigned short> PortMap;

int main(int argc, char const *argv[]) {
  try {
    slip::Udp udp;
    slip::Tcp tcp;
    MessagesMap message_map;
    PortMap port_map;
    unsigned short udp_port = atoi(argv[1]);
    unsigned short tcp_port = atoi(argv[2]);
    udp.add_listener(udp_port, [&tcp, &tcp_port, &message_map, &port_map](std::string source_ip, unsigned short source_port, std::string message){
      std::stringstream parser(message);
      std::string cmd;
      parser >> cmd;
      if (cmd == "LGIN") {
        std::string username;
        unsigned short int port;
        parser >> username >> port;
        #ifdef DEBUG
          std::cout << "LOGIN: " << username << std::endl;
        #endif
        if(message_map.find(username) == message_map.end()) {
          message_map[username] = std::queue<std::string>();
        }
        if(message_map.find(username) == message_map.end()) {
          message_map[username] = std::queue<std::string>();
        }
      } else if (cmd == "LGOU") {
        std::string username;
        parser >> username;
        #ifdef DEBUG
          std::cout << "LOGOUT: " << username << std::endl;
        #endif
        message_map.erase(username);
      } else if (cmd == "LIST") {
        unsigned short int port;
        parser >> port;
        #ifdef DEBUG
          std::cout << "LIST: send all online user to " << source_ip << ":" << port << std::endl;
        #endif
        auto pcb = tcp.connect(source_ip, port, tcp_port);
        for(MessagesMap::iterator it = message_map.begin(); it != message_map.end(); ++it) {
          pcb->send(it->first);
        }
        pcb->close();
        while(pcb->state != slip::Tcp::CLOSED) {};
      } else if (cmd == "SEND") {
        std::string source_username, dest_username, content;
        parser >> source_username >> dest_username;
        std::getline(parser, content);
        std::string message = (source_username + ": ") + content;
        message_map[dest_username].push(message);
        #ifdef DEBUG
          std::cout << "SEND: push \"" << message << "\" TO " << dest_username << std::endl;
        #endif
      } else if (cmd == "PULL") {
        std::string username;
        unsigned short int port;
        parser >> username >> port;
        #ifdef DEBUG
          std::cout << "PULL all message sent to " << username << " TO: " << source_ip << ":" << port << std::endl;
        #endif         
        auto pcb = tcp.connect(source_ip, port, tcp_port);
        if(message_map.find(username) != message_map.end()) {
          std::string message;
          MessageList &message_list = message_map[username];
          while(!message_list.empty()) {
            message = message_list.front();
            message_list.pop();
            pcb->send(message);
            #ifdef DEBUG
              std::cout << "  MESSAGE: \"" << message << "\"" << std::endl;
            #endif
          }
        } else {
          std::string message = "user not exist";
          pcb->send(message);
          #ifdef DEBUG
            std::cout << "    MESSAGE: \"" << message << "\"" << std::endl;
          #endif
        }
        pcb->close();
        while(pcb->state != slip::Tcp::CLOSED) {};
      }
    });
    while(1) {};
  } catch (std::runtime_error e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}