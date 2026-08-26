# CLAUDE.md - coroutine 模块

## 模块定位

`coroutine` 是基于 event 架构的协程库，用于处理事件驱动模型不擅长的**顺序型业务逻辑**。详见 `README.md`。

## 依赖关系

- 上游依赖：`event`、`base`
- 被依赖：main（Context 暴露 `coroutine::Scheduler`）、run

## 关键组件

| 文件 | 说明 |
|------|------|
| `scheduler.h` | `Scheduler` 协程调度器：`create()` 创建协程（返回 `RoutineToken`）、`resume()`/`cancel()`；协程内 `yield()`/`wait()`/`join()`；主协程 `cleanup()` |
| `channel.hpp` | `Channel` 通道，多协程等待数据，最早等待者被唤醒（类似 golang 的 ch） |
| `semaphore.hpp` | `Semaphore` 信号量 |
| `mutex.hpp` | `Mutex` 互斥量 |
| `condition.hpp` | `Condition` 条件量（多条件同时/任意满足唤醒） |
| `broadcast.hpp` | `Broadcast` 广播，多个协程等待同一信号 |

## 为什么用协程

- 线程偏重、切换不可控、资源抢占难管理；
- 协程轻量（每协程一个栈，栈大小可指定 `ROUTINE_STACK_DEFAULT_SIZE = 8192`）、主动切换（`yield()`/`wait()`）、无资源抢占问题。

## 基础用法

```c++
event::Loop *loop = event::Loop::New();
coroutine::Scheduler sch(loop);
auto entry = [&](coroutine::Scheduler &sch) { /* ... sch.yield(); ... */ };
sch.create(entry);          // 创建并立即运行
loop->runLoop();
```

## 注意事项

- 协程在主 Loop 线程上调度，不能在其中执行阻塞操作。
- 辅助组件（Channel/Semaphore/Mutex/Condition/Broadcast）具体用法见各 `*_test.cpp`。

## 测试

- 测试文件：`scheduler_test.cpp`、`channel_test.cpp`、`semaphore_test.cpp`、`mutex_test.cpp`、`broadcast_test.cpp`、`condition_test.cpp`
- 运行：`.build/coroutine/test`
