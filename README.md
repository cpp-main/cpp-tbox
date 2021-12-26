# cpp\_tbox

#### 介绍
cpp\_tbox，全称: C++ Treasure Box，C++开发百宝箱，是基于事件的服务型应用开发库。

#### 依赖包安装
apt -y install g++ make libgtest libevent-dev libev-dev libgtest-dev

#### 适用环境

- Linux 环境，主要是针对服务型应用的；
- C++11 以上，都2021年了，C++11之前的古老版本就让它进坟墓吧。

#### 模块介绍

- base，基础库，含日志打印、常用工具等；
- util，工具库，在业务代码中可能会用到的库；
- event，事件库，对 libevent2, libev 库进行了统一的封装，实现Fd,Timer,Signal三种事件驱动；
- eventx，事件扩展库，含 ThreadPool 线程池模块，专用于处理阻塞性事务；
- network，网络库，实现了串口、终端、UDP、TCP 通信模块；
- coroutine，协程库，众所周知，异步框架不方便处理顺序性业务，协程弥补之；
- mqtt，MQTT客户端库；
- main，应用程序框架，实现了完备的程序启动流程与框架，让开发者只需关心业务代码；

#### 模块间依赖

- base --> None
- util --> base
- event --> base, [libevent\_core, libev]
- eventx --> base, event, [pthread]
- network --> base, event
- coroutine --> base, event
- mqtt --> base, event, [mosquitto]
- main --> base, util, event, eventx

#### 未来规化

- 创建 http 模块，实现 Http 相关的 Server 与 Client 端；
- 在 network 中支持 TLS；
- 实现异步日志输出模块；
- 实现类似于 shell 的命令交互，并添加到 main 中；
- 实现使用 CMake 进行工程管理；
- 创建 CoAP 模块，实现 CoAP 的客户端与服务端；
