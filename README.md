##  学习boost/asio网络编程。
首先声明一点的是，这所有的文件夹里的源码都是需要链接boost库的，否则无法运行程序。(建议使用VS2022，VScode/CLion搭配CMake的话也可以)
***
1. tcp_client.cpp就是最基础的同步客户端源码，而tcp_server.cpp则是最基础的服务端源码。至于async_server.cpp到4，5，...就是异步服务器不断迭代，升级的过程。为啥不用socket那一套，太底层了，boost已经为我们封装好了，功能齐全又简便。
---
2. 如你所见，这是关于boost库的网络编程的学习记录.学习资源来源于恋恋风辰的博客。