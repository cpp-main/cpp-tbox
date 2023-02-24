# cpp-tbox

# 介绍
cpp-tbox，全称: C++ Treasure Box，C++开发百宝箱，是基于事件的服务型应用开发库。

如果您需要使用C++开发服务型软件，您使用它可以助你事半功倍。因为这里有完备的软件框架与服务型软件开发配套成体系的工具集，拿来即用。使用它打造一个服务型的软件非常简单。

打个比方：如果你想要建一栋别墅。你会怎么做呢？如果你从零修建它，就太费时费力了。你可能要自己打地基、烧砖，砌墙、封顶，而且最终修建出来的东西还不一定稳固，可能还会漏雨、甚至倒塌。好在这里就有一个成熟且免费的施工方案。它有栋现成的经过了工程检验了的新别墅，里面水电气网全通，并配备了各种家具：灶、洗衣机、冰箱等供你选择。基于实用方面的考虑，为何不用现成的？

cpp-tbox就提供了这样的别墅与配套设施。所谓的现成的别墅就是cpp-tbox中的main框架。水电气网，就是这个main框架内置的事件驱动、日志系统、参数系统、调试终端、线程池模块。而所谓的家具就是cpp-tbox中其它常用模块，如：通信（TCP,UDP,HTTP,MQTT,UART等）、协程、闹钟、序列化、状态机、行为树、AES、MD5等等，几乎网络编程需要的模块，这里都有。
有了cpp-tbox，打造一个服务型程序就变得非常简单多了：在main框架的基础上，编写业务模块就可以了，连`main()`函数都不用写。省去了大量没必要的开发工作量。

真的很好用，快来试试吧！

# 适用环境

- Linux 环境，主要是针对服务型应用的；
- C++11 以上。

# 模块介绍

- base，基础库，含日志打印、常用工具等；
- util，工具库，在业务代码中可能会用到的库；
- event，事件库，实现Fd,Timer,Signal三种事件驱动；
- log，日志输出库，实现了终端、syslog、文件形式的日志输出；
- eventx，事件扩展库，含 ThreadPool 线程池模块，专用于处理阻塞性事务；TimerPool 定时器池模块；
- network，网络库，实现了串口、终端、UDP、TCP 通信模块；
- http，HTTP库，在network的基础上实现了HTTP的Server与Client模块；
- coroutine，协程库，众所周知，异步框架不方便处理顺序性业务，协程弥补之；
- mqtt，MQTT客户端库；
- terminal, 终端，类似shell的命令终端，可实现运行时与程序进行命令交互；
- main，应用程序框架，实现了完备的程序启动流程与框架，让开发者只需关心业务代码；
- alarm，闹钟模块，实现了4种常用的闹钟：CRON闹钟、单次闹钟、星期循环闹钟、工作日闹钟；
- flow，流程模块，含多层级状态机与行为树，解决异步模式下动行流程问题；

# 外部库依赖

| 库名 | 依赖模块 | 说明 | 安装方法 |
|:----:|:--------:|:----:|:--------:|
| googletest | 所有模块 | 单元测试用，如果不进行单元测试可忽略 | sudo apt install google-mock |
| libevent | event | 默认不依赖，在event/config.mk中开启了WITH\_LIBEVENT时依赖 | sudo apt install libevent-dev |
| libev | event | 默认不依赖，在event/config.mk中开启了WITH\_LIBEV时依赖 | sudo apt install libev-dev |
| mosquitto | mqtt | MQTT库 | sudo apt install libmosquitto-dev |
| nlohmann/json | main | 作为配置数据用 | 下载[json\_fwd.hpp](https://raw.githubusercontent.com/nlohmann/json/v3.10.4/include/nlohmann/json_fwd.hpp)与[json.hpp](https://raw.githubusercontent.com/nlohmann/json/v3.10.4/single_include/nlohmann/json.hpp)到头文件目录，如：/usr/local/include/nlohmann/ |

**安装命令**

| 系统 | 安装命令 |
|:----:|:------:|
| Ubuntu/Debian | `apt install -y g++ make google-mock libevent-dev libev-dev libmosquitto-dev` |

**如果构建**

进入到cpp-tbox的顶层目录，执行命令:  
```
STAGING_DIR=$HOME/.local make modules RELEASE=1
```

完成之后，所有的头文件导出到 `$HOME/.local/include/`，所有的库文件输出到 `$HOME/.local/lib/`。
如果你没有指定 `STAGING_DIR` 参数，它默认为 `.staging`。

在你自己工程的 Makefile 中，你需要添加以下的内容:
```
CXXFLAGS += -I$(HOME)/.local/include
LDFLAGS += -L$(HOME)/.local/lib -ltbox_xxxxx
```
然后就可以使用tbox的组件了。

# 模块间依赖

![](http://assets.processon.com/chart_image/6227efafe401fd18bcfc83e8.png)

# 模块裁减

打开 config.mk 文件，将不需要模块对应 `app_y += xxx` 屏蔽即可，但要注意模块间的依赖性。

# 未来规划

- 实现IPv6
- 实现TLS的支持

