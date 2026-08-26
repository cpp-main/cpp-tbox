# CLAUDE.md - eventx 模块

## 模块定位

`eventx` 是 event 的扩展模块，在事件循环基础上提供多线程执行、定时器管理、异步任务等能力。

## 依赖关系

- 上游依赖：`event`、`util`、`base`
- 被依赖：log、network、main、http、flow、jsonrpc、network_tls 等

## 关键组件

| 文件 | 说明 |
|------|------|
| `thread_executor.h` | `ThreadExecutor` 线程执行器接口（`execute()` 返回 `TaskToken`，含任务状态/取消） |
| `thread_pool.h` | `ThreadPool` 线程池（常驻/最大线程数，优先级 `THREAD_POOL_PRIO_MIN..MAX`） |
| `work_thread.h` | `WorkThread` 工作线程（单线程、无优先级的精简版线程池） |
| `timer_pool.h` | `TimerPool` 定时任务管理器（`doEvery`/`doAfter`/`doAt`，返回 `TimerToken`） |
| `timer_fd.h` | `TimerFd` 基于 timerfd 的定时器（纳秒精度） |
| `async.h` | `Async` 将阻塞调用转异步（文件读写、执行命令） |
| `loop_thread.h` | `LoopThread` 独立运行 Loop 的线程 |
| `loop_wdog.h` | `LoopWDog` Loop 看门狗（监控 Loop 线程阻塞） |
| `timeout_monitor.hpp` | `TimeoutMonitor<T>` 超时监控器（模板，配合 Cabinet 实现请求池） |
| `request_pool.hpp` | `RequestPool<T>` 请求池（存储请求上下文 + 超时自动处理） |

## 注意事项

- `ThreadExecutor`/`WorkThread` 的任务回调默认在主 Loop 线程执行；`execute(backend_task, main_cb)` 的第二参数用于指定完成后的主线程回调。
- `TimerPool` 使用 `cabinet::Token` 作为定时器句柄，注意「对象生命期倒挂」问题——回调持有短命对象时须先 `cancel()`。
- `TimeoutMonitor` / `RequestPool` 是纯头文件模板。
- `Async` 依赖 `ThreadPool`，用于避免阻塞 Loop 线程。

## 测试

- 测试文件：thread_pool、timer_pool、timeout_monitor、request_pool、loop_wdog、work_thread、loop_thread、timer_fd、async 各自的 `*_test.cpp`
- 运行：`.build/eventx/test`

## 示例

- `examples/eventx/thread_pool`、`examples/eventx/timer_fd`
