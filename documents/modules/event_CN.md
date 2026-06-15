# 事件驱动模块 (event)

## 是什么？

event 模块是 cpp-tbox 的核心基石，提供了事件循环（Loop）与三种基本事件类型（FdEvent、TimerEvent、SignalEvent），是整个框架运转的心脏。几乎所有其他模块都依赖 event 模块运行。

## 为什么需要它？

在服务型程序中，程序需要同时响应多种事件：网络数据到达、定时任务到期、信号中断等。如果用传统多线程处理，会遇到线程同步复杂、资源抢占等问题。事件驱动模型通过单线程事件循环，高效地处理所有异步事件，避免线程切换开销。

## 头文件

```cpp
#include <tbox/event/loop.h>          //! 事件循环
#include <tbox/event/fd_event.h>      //! 文件描述符事件
#include <tbox/event/timer_event.h>   //! 定时器事件
#include <tbox/event/signal_event.h>  //! 信号事件
#include <tbox/event/event.h>         //! 事件基类
```

## 核心类与接口

### Loop — 事件循环

事件循环是所有事件处理的调度中心。程序通过 `runLoop()` 进入循环，在循环中监听和分发事件。

| 方法 | 说明 |
|------|------|
| `Loop::New()` | 创建默认类型的事件循环 |
| `Loop::New(engine_type)` | 创建指定引擎类型的事件循环（如 "epoll"、"poll"） |
| `Loop::Engines()` | 获取可用的引擎列表 |
| `runLoop(Mode)` | 运行事件循环，Mode::kOnce 执行一次，Mode::kForever 持续运行 |
| `exitLoop(wait_time)` | 退出事件循环，可指定等待时间 |
| `isInLoopThread()` | 判断是否在 Loop 线程内 |
| `isRunning()` | 判断 Loop 是否正在运行 |
| `newFdEvent(what)` | 创建 FdEvent 对象 |
| `newTimerEvent(what)` | 创建 TimerEvent 对象 |
| `newSignalEvent(what)` | 创建 SignalEvent 对象 |
| `cleanup()` | 清理 Loop 资源 |

#### 任务注入方法

Loop 提供三种将函数注入循环执行的方法，它们的区别如下：

| 方法 | 特点 | 适用场景 |
|------|------|----------|
| `runInLoop(func)` | 有加锁操作，支持跨线程、跨 Loop 调用 | 不同 Loop 之间委派任务，或其它线程向 Loop 线程派任务 |
| `runNext(func)` | 无加锁操作，不支持跨线程，仅 Loop 线程内调用 | 在当前回调完成后立即执行，如释放对象自身 |
| `run(func)` | 自动选择：Loop 线程内用 runNext，否则用 runInLoop | 不确定该用哪个时，选它即可 |

> **使用建议**：明确在 Loop 线程内的用 `runNext()`，明确不在 Loop 线程内的用 `runInLoop()`，不清楚的用 `run()`。

所有注入方法返回 `RunId`，可通过 `cancel(RunId)` 取消未执行的任务。

### FdEvent — 文件描述符事件

监听文件描述符（fd）上的可读、可写、异常事件。

| 方法 | 说明 |
|------|------|
| `initialize(fd, events, mode)` | 初始化，events 为 kReadEvent/kWriteEvent/kExceptEvent 组合 |
| `setCallback(cb)` | 设置回调函数，参数为 `short events` |
| `enable()` | 启用监听 |
| `disable()` | 停用监听 |

Event 模式：
- `Mode::kPersist` — 持续监听，事件触发后不自动取消
- `Mode::kOneshot` — 一次性监听，事件触发后自动取消

### TimerEvent — 定时器事件

在指定时间后触发回调。

| 方法 | 说明 |
|------|------|
| `initialize(time_span, mode)` | 初始化，time_span 为毫秒时长 |
| `setCallback(cb)` | 设置回调函数 |
| `enable()` | 启用定时器 |
| `disable()` | 停用定时器 |

### SignalEvent — 信号事件

监听 Linux 信号（如 SIGINT、SIGTERM）。

| 方法 | 说明 |
|------|------|
| `initialize(signum, mode)` | 初始化，监听单个信号 |
| `initialize(sigset, mode)` | 初始化，监听信号集合 |
| `initialize({sig1,sig2,...}, mode)` | 初始化，监听信号列表 |
| `setCallback(cb)` | 设置回调函数，参数为 `int signum` |

## 使用示例

### 定时器示例

> 完整示例见 `examples/event/02_timer/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

using namespace tbox;
using namespace tbox::event;

int main() {
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    //! 创建周期定时器，每秒触发一次
    auto sp_timer = sp_loop->newTimerEvent("timer");
    SetScopeExitAction([sp_timer] { delete sp_timer; });

    sp_timer->initialize(std::chrono::seconds(1), Event::Mode::kPersist);
    sp_timer->setCallback([] { LogInfo("timer tick"); });
    sp_timer->enable();

    //! 运行5秒后退出
    sp_loop->exitLoop(std::chrono::seconds(5));
    sp_loop->runLoop();

    LogOutput_Disable();
    return 0;
}
```

### 信号处理示例

> 完整示例见 `examples/event/03_signal/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

using namespace tbox;
using namespace tbox::event;

int main() {
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    //! 监听 SIGINT 与 SIGTERM 信号
    auto sp_sig = sp_loop->newSignalEvent("signal");
    SetScopeExitAction([sp_sig] { delete sp_sig; });

    sp_sig->initialize({SIGINT, SIGTERM}, Event::Mode::kPersist);
    sp_sig->setCallback(
        [sp_loop] (int signum) {
            LogInfo("received signal %d", signum);
            sp_loop->exitLoop();
        }
    );
    sp_sig->enable();

    sp_loop->runLoop();
    LogOutput_Disable();
    return 0;
}
```

### runInLoop 跨线程任务注入

> 完整示例见 `examples/event/04_run_in_loop/`

```cpp
//! 在其它线程中向 Loop 注入任务
std::thread t(
    [sp_loop] {
        //! 跨线程安全调用
        sp_loop->runInLoop([] { LogInfo("task from other thread"); });
    }
);
```

### runNext 释放对象自身

> 完整示例见 `examples/event/07_delay_delete/`

```cpp
//! 在回调中安全释放自身对象
void MyClass::onTimeout() {
    //! 不能直接 delete this，会导致回调中访问已析构对象
    //! 使用 runNext 在回调完成后执行释放
    loop_->runNext([this] { delete this; });
}
```

## 常见场景

1. **程序主循环**：创建 Loop，运行 `runLoop(Mode::kForever)`，通过 `exitLoop()` 退出
2. **定时任务**：使用 TimerEvent 创建周期或一次性定时器
3. **信号处理**：使用 SignalEvent 捕获 SIGINT/SIGTERM，优雅退出程序
4. **跨线程通信**：其它线程通过 `runInLoop()` 向 Loop 线程注入任务
5. **安全释放对象**：通过 `runNext()` 在回调结束后释放对象

## 注意事项

1. **Loop 是单线程的**：所有事件回调都在 Loop 线程中执行，不要在回调中执行耗时操作，否则会阻塞事件循环
2. **runNext 仅限 Loop 线程**：禁止跨线程调用 `runNext()`，跨线程必须使用 `runInLoop()`
3. **事件对象生命周期**：通过 Loop 创建的事件对象（newFdEvent/newTimerEvent/newSignalEvent）需要手动 delete，建议使用 `SetScopeExitAction` 管理生命周期
4. **Oneshot 模式**：TimerEvent 的 kOneshot 模式触发后自动 disable，如需再次触发需重新 enable
5. **cancel() 的时机**：已开始执行的任务无法 cancel，只能取消尚未执行的任务

## 相关模块

- **eventx**：基于 event 提供线程池、定时池等高级功能
- **base**：提供日志宏、ScopeExit 等基础设施
- **network**：基于 event 的 FdEvent 实现 TCP/UDP/UART 通信
- **alarm**：基于 event 的 TimerEvent 实现定时闹钟
- **main**：框架内置 Loop 对象，通过 Context 获取

## 参考图片

![tbox-loop](../images/0001-tbox-loop.jpg)
