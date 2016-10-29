# 简易TCP/IP与UDP接口实现（Slip）

Super Lightweight Internet Protocol Inplementation based on C++.

项目组成员：

* 郑齐(@tidyzq) 
* 赵子琳(@SakurazukaKen)
* 欧一锋(@a20185)
* 王毅峰
* 王天宇



## 项目目录结构：

TCP（主目录）

* README.md
* Makefile
* *bin*  (存放此实现的测试类编译后生成的**二进制文件**)
  * tcptest
  * udptest
* *build* (存放此实现编译过程中生成的**.o文件**)
  * tcp.o
  * *test*
    * tcptest.o
    * udptest.o
  * udp.o
  * utils.o
* *demo* (存放基于此实现的一个**小应用**)
  * *include* (存放实现**小应用**的所有需要用到的C++头文件)
  * *src* (存放**小应用**的所有C++文件)
  * *bin* (存放实现**小应用**编译后生成的**二进制文件**)
  * Makefile
* *include* (存放此实现的所有需要用到的C++头文件)
  * tcp.hpp
  * udp.hpp
  * utils.hpp
* *src* (存放基于此实现的所有C++文件)
  * tcp.hpp
  * udp.hpp
  * utils.hpp
* test(存放基于此实现的测试类的C++文件)
  * tcptest.cpp
  * udptest.cpp




## 使用方式:

* Shell 下进入本目录执行`make`命令即可编译。
* 执行测试可以使用`bin`目录下的*tcptest*以及 *udptest* 文件。
* 测试文件使用方法：
  * ./tcptest --mode  --dest-ip  --dest-port  --source-port   
    * *mode*:指定tcp使用模式。**s**为发送，**l**为接收。（注意，如果为接收模式的话，则*dest-ip*以及*dest-port*为空。）
    * *dest-ip*:目的主机的ip地址。
    * *dest-port*:目的主机的端口。
    * *source-port*:本地主机的端口。
  * ./udptest  --dest-ip  --dest-port  --source-port  --dgpkt-count
    * *dest-ip*:目的主机的ip地址。
    * *dest-port*:目的主机的端口。
    * *source-port*:本地主机的端口。
    * *dgpkt-count*: 测试发送的数据报文包(*datagram packet*)数量。

 

  

## 本实现的API接口：

### Utils类：

 (通过*命名空间* 调用即可，本类置于*namespace slip* 中)

* **校验和计算方法：**

  * 使用方式:

  * ```c++
    unsigned short calc_checksum(unsigned long source_ip, 
                  unsigned long dest_ip, 
                  u_int8_t protocol,
                  char* payload, 
                  unsigned short payload_len);
    ```

    * *source_ip*: 源主机ip
    * *dest_ip*：目的主机ip
    * *protocol*： 8位协议号
    * *payload*:   数据包
    * *payload_len*: 数据包长度

     

* **校验和检查方法:**

  * 使用方式:

  * ```c++
    bool verify_checksum(unsigned long source_ip, 
                         unsigned long dest_ip, 
                         u_int8_t protocol,
                         char* payload,
                         unsigned short payload_len,
                         unsigned short checksum);
    ```

    * *source_ip*:    源主机ip地址
    * *dest_ip*:    目的主机ip地址
    * *protocol*:    8位协议号
    * *payload*:   数据包
    * *payload_len*:   数据包长度
    * *checksum*:    传入的原始报文校验和

 

* 获取本地IP方法：

  * 使用方法：

  * ```c++
    std::string get_local_ip();
    ```

    ​

 

### UDP类

* ​