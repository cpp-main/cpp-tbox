# 简介
cpp-tbox,全称: C++ Treasure Box,是基于事件的服务型应用开发库以及开发框架。

# 特点

1.基于事件驱动的Reactor模型
2.开箱即用
2.具有类Shell的终端
3.完备的日志系统
4.灵活的参数系统
5.支持线程池
6.支持主次线程间无锁传递
7.优雅的退出流程
7.丰富的组件

# 应用场景

- 智能硬件(机器人、智能家居、机顶盒、无人机、车载等);
- 后台服务软件;

# 适用环境

- Linux 环境,主要是针对服务型应用的;
- C++11 以上。

# 安装与构建
[安装与构建教程](documents/00.install.md)

# 快速上手
[快速上手教程](documents/01.quick_start.md)


# 模块介绍

- base,基础库,含日志打印、常用工具等;
- util,工具库,在业务代码中可能会用到的库;
- event,事件库,实现Fd,Timer,Signal三种事件驱动;
- log,日志输出库,实现了终端、syslog、文件形式的日志输出;
- eventx,事件扩展库,含 ThreadPool 线程池模块,专用于处理阻塞性事务;TimerPool 定时器池模块;
- network,网络库,实现了串口、终端、UDP、TCP 通信模块;
- http,HTTP库,在network的基础上实现了HTTP的Server与Client模块;
- coroutine,协程库,众所周知,异步框架不方便处理顺序性业务,协程弥补之;
- mqtt,MQTT客户端库;
- terminal, 终端,类似shell的命令终端,可实现运行时与程序进行命令交互;
- main,应用程序框架,实现了完备的程序启动流程与框架,让开发者只需关心业务代码;
- alarm,闹钟模块,实现了4种常用的闹钟:CRON闹钟、单次闹钟、星期循环闹钟、工作日闹钟;
- flow,流程模块,含多层级状态机与行为树,解决异步模式下动行流程问题;

# 外部库依赖

| 库名 | 依赖模块 | 说明 | 安装方法 |
|:----:|:--------:|:----:|:--------:|
| libgtest-dev | 所有模块 | 单元测试用,如果不进行单元测试可忽略 | sudo apt install libgtest-dev |
| libgmock-dev | 所有模块 | 单元测试用,如果不进行单元测试可忽略 | sudo apt install libgmock-dev |
| mosquitto | mqtt | MQTT库,如果不使用mqtt模块可忽略 | sudo apt install libmosquitto-dev |

# 模块间依赖

![](/documents/images/modules-dependence.png)

# 模块裁减

打开 config.mk 文件,将不需要模块对应 `app_y += xxx` 屏蔽即可,但要注意模块间的依赖性。

# 联系我

- Issue:使用任何问题都欢迎在issue里交流
- 微信:hevake_lee
- QQ群:738084942(cpp-tbox 技术交流)