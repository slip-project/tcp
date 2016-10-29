# 简易TCP/IP与UDP接口实现（Slip）

Super Lightweight IP Inplementation base on C++.

项目组成员：郑齐(@tidyzq) 、赵子琳(@SakurazukaKen)、欧一锋(@a20185)、王毅峰、王天宇



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



使用方式:

* Shell 下进入本目录执行`make`命令即可编译。