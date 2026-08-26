# CLAUDE.md - event 模块

## 模块定位

`event` 是框架核心的事件驱动（Reactor）模块，提供事件循环 `Loop` 及三类事件：文件描述符事件、定时器事件、信号事件。

## 依赖关系

- 上游依赖：`base`
- 被依赖：eventx、log、network、terminal、coroutine、main、http、mqtt、flow、alarm、dbus、jsonrpc 等

## 关键组件

| 文件 | 说明 |
|------|------|
| `loop.h` | `Loop` 事件循环抽象：`runLoop()`/`exitLoop()`、`runInLoop()`/`runNext()`/`run()`、`newFdEvent()`/`newTimerEvent()`/`newSignalEvent()`、统计 `Stat`、水位线 `WaterLine` |
| `event.h` | `Event` 基类（`enable`/`disable`/`isEnabled`、`kPersist`/`kOneshot` 模式） |
| `fd_event.h` | `FdEvent` 文件描述符事件（读/写/异常） |
| `timer_event.h` | `TimerEvent` 定时器事件 |
| `signal_event.h` | `SignalEvent` 信号事件（支持信号集） |
| `stat.h` | `Stat` 循环统计信息 |
| `forward.h` | 前置声明 |

### 引擎

- `engines/epoll/` — Linux epoll 引擎（默认，`HAVE_EPOLL` 控制）
- `engines/select/` — select 引擎（兜底）

### 实现文件

- `common_loop*` — Loop 通用逻辑拆分（`common_loop.cpp`、`common_loop_run.cpp`、`common_loop_timer.cpp`、`common_loop_signal.cpp`）
- `timer_event_impl.*` / `signal_event_impl.*` — 事件实现

## 核心概念：三种委派方式

- `runInLoop()`：注入下一轮执行，加锁，支持跨线程/跨 Loop。
- `runNext()`：本回调完成后立即执行，无锁，仅 Loop 线程内。
- `run()`：自动选择——同线程用 `runNext()`，否则用 `runInLoop()`。

## 注意事项

- `Loop` 用 `Loop::New()`（默认引擎）或 `Loop::New(engine_type)` 创建，析构前应调用 `cleanup()`。
- Loop 线程内禁止阻塞操作（可用 `eventx::LoopWDog` 监控阻塞）。
- 信号处理依赖 `signal_event.cpp` 内部实现，跨平台需注意。

## 测试

- 测试文件：`common_loop_test.cpp`、`fd_event_test.cpp`、`timer_event_test.cpp`、`signal_event_test.cpp`
- 运行：`.build/event/test`
