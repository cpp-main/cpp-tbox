# 简介
cpp-tbox,全称: C++ Treasure Box,C++百宝箱,是一个基于 Reactor 模式的服务型 **开发框架** 与 **组件库**。

# 特点

## 1. 基于Reactor模型
参考 Node.js 的 Reactor 模式。主线程以 Reactor 模式处理非阻塞 IO 事件,并配合 ThreadPool 执行大运算与阻塞性操作。

该模式避免了多线程模式竞态加锁的烦恼,程序稳定可靠。

## 2. 开箱即用
内含 main 框架可直接构建出可执行程序,不需要使用者编写 `main()` 程序入口。  
一切都已准备好,您只需要填写业务代码即可。

## 3. 具有类Shell的命令终端
可以与运行中的服务通过telnet进行交互,令其打印内部数据,或是执行特定的动作。
![](documents/images/0000-terminal-show.gif)
这极大地降低了调试难度。

## 4. 完备的日志系统
**1) 有三种日志输出渠道:终端 + 文件 + syslog**

**2) 根据日志等级渲染不同颜色,一目了然,内容详尽**
![](documents/images/0002-log-show.png)
日志内容包含了:等级、时间(精确到微秒)、线程号、模块名、函数名、正文、文件名、行号。
方便快速定位问题。

**3) 灵活的日志输出过滤器,且能运行时修改**
可针对不同的模块单独设置日志等级。

## 5. 灵活的参数系统,以不变应万变
![](documents/images/0005-arguments.png)

## 6. 支持线程池、主次线程间无锁传递
子线程委托主线程执行:
![](documents/images/0003-run-in-loop.png)

主线程委托子线程执行:
![](documents/images/0004-run-thread-pool.png)

## 7. 支持优雅的退出流程
在接收到信号:SIGINT, SIGTERM, SIGQUIT, SIGPWR 时,会有序地执行退出流程,释放资源。做到干净地退出。

## 8. 有全面的异常捕获机制
当程序出现各种程序异常,如:段错误、断言、总线错误、异常未捕获等,架框会捕获并在日志系统中打印完整的调用栈,如:
![](documents/images/0006-error-dump.png)
面对程序蹦溃,不再一脸茫然。

## 9. 有丰富的开发组件

| 库名 | 中文名 | 说明 |
|:----:|:---:|:----|
| base | 基础库 | 含日志打印、常用工具等 |
| util | 工具库 | 在业务代码中可能会用到的库 |
| event | 事件库 | 实现了IO,Timer,Signal三种事件驱动,是整个框架的心脏 |
| eventx | 事件扩展库 | 含 ThreadPool 线程池,WorkThread工作线程,TimerPool 定时器池等模块 |
| log | 日志输出库 | 实现了终端、syslog、文件形式的日志输出 |
| network | 网络库 | 实现了串口、终端、UDP、TCP 通信模块 |
| terminal | 终端 | 类似shell的命令终端,可实现运行时与程序进行命令交互 |
| **main** | 主框架 | 实现了完备的程序启动流程与框架,让开发者只需关心业务代码 |
| mqtt | MQTT客户端库 | |
| coroutine | 协程库 | 众所周知,异步框架不方便处理顺序性业务,协程弥补之 |
| http | HTTP库 | 在network的基础上实现了HTTP的Server与Client模块 |
| alarm | 闹钟模块 | 实现了4种常用的闹钟:CRON闹钟、单次闹钟、星期循环闹钟、工作日闹钟 |
| flow | 流程模块 | 含多层级状态机与行为树,解决异步模式下动行流程问题 |

# 应用场景

- 智能硬件(如:机器人、智能家居、机顶盒、无人机、车载等);
- 边缘计算组件;
- 后台服务型软件;

# 适用环境

- Linux 操作系统;
- C++11 以上。

# 安装与构建
[安装与构建教程](documents/00.install.md)

# 快速上手
[快速上手教程](documents/01.quick_start.md)

# 外部库依赖

| 库名 | 依赖模块 | 说明 | 安装方法 |
|:----:|:--------:|:----:|:--------:|
| libgtest-dev | 所有模块 | 单元测试用,如果不进行单元测试可忽略 | sudo apt install libgtest-dev |
| libgmock-dev | 所有模块 | 单元测试用,如果不进行单元测试可忽略 | sudo apt install libgmock-dev |
| mosquitto | mqtt | MQTT库,如果不使用mqtt模块可忽略 | sudo apt install libmosquitto-dev |

# 模块间依赖

![](documents/images/modules-dependence.png)

# 模块裁减

打开 config.mk 文件,将不需要模块对应 `app_y += xxx` 屏蔽即可,但要注意模块间的依赖性。

# 开源许可

[MIT](LICENSE),可免费商用。

# 反馈途径

- Issue:任何问题都欢迎在issue里交流
- 微信:hevake_lee
- QQ群:738084942(cpp-tbox 技术交流)
