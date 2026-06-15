# 事件扩展模块 (eventx)

## 是什么？

eventx 模块基于 event 模块提供高级异步编程组件：线程池（ThreadPool）、定时池（TimerPool）、独立 Loop 线程（LoopThread）、异步操作（Async）、超时监控（TimeoutMonitor）和请求池（RequestPool）。这些组件让开发者更方便地处理多线程协作、定时任务管理、请求超时等复杂场景。

## 为什么需要它？

event 模块提供了单线程事件循环，但在实际应用中常需要：
- 将耗时的计算或 I/O 操作放到后台线程执行，完成后回到主线程处理结果
- 创建大量定时器但不想逐个管理 TimerEvent 的生命周期
- 在独立线程中运行另一个事件循环
- 将阻塞性的系统调用（如文件读写）转换为异步回调形式

eventx 正是为了解决这些问题而设计的。

## 头文件

```cpp
#include <tbox/eventx/thread_pool.h>       //! 线程池
#include <tbox/eventx/timer_pool.h>        //! 定时池
#include <tbox/eventx/loop_thread.h>       //! 独立 Loop 线程
#include <tbox/eventx/async.h>             //! 异步操作
#include <tbox/eventx/request_pool.hpp>     //! 请求池
#include <tbox/eventx/timeout_monitor.hpp> //! 超时监控
#include <tbox/eventx/thread_executor.h>   //! 线程执行器接口
```

## 核心类与接口

### ThreadPool — 线程池

线程池用于将耗时任务委派给后台线程执行，并在完成后回到主线程执行回调。

| 方法 | 说明 |
|------|------|
| `ThreadPool(main_loop)` | 构造，指定主线程的 Loop |
| `initialize(min, max)` | 初始化，指定常驻线程数与最大线程数 |
| `execute(task, prio)` | 在 worker 线程执行任务，prio 为优先级 [-2,2] |
| `execute(task, main_cb, prio)` | worker 线程执行 task，完成后主线程执行 main_cb |
| `execute(task)` | 在 worker 线程执行任务（无回调） |
| `execute(task, main_cb)` | worker 线程执行 task，完成后主线程执行 main_cb |
| `getTaskStatus(token)` | 获取任务状态 |
| `cancel(token)` | 取消任务 |
| `cleanup()` | 清理资源，等待所有 worker 线程结束 |
| `snapshot()` | 获取线程池快照（线程数、空闲数、任务数等） |

### TimerPool — 定时池

定时池让开发者轻松创建定时任务，无需关心 TimerEvent 的生命周期管理。

| 方法 | 说明 |
|------|------|
| `TimerPool(loop)` | 构造，指定 Loop |
| `doEvery(msec, cb)` | 周期性定时任务，返回 TimerToken |
| `doAfter(msec, cb)` | 一次性延迟任务，返回 TimerToken |
| `doAt(time_point, cb)` | 在指定时间点执行，返回 TimerToken |
| `cancel(token)` | 取消定时任务 |
| `cleanup()` | 清理所有定时器 |

> **注意**：使用 `cancel()` 取消定时器时，要确保回调函数中持有的对象仍然存活，避免生命期倒挂问题。

### LoopThread — 独立 Loop 线程

在独立线程中运行一个事件循环，常用于需要在另一个线程处理事件的场景。

| 方法 | 说明 |
|------|------|
| `LoopThread(run_now, name)` | 构造，指定是否立即运行和 Loop 名称 |
| `start()` | 启动线程 |
| `stop()` | 停止线程 |
| `isRunning()` | 线程是否正在运行 |
| `loop()` | 返回 Loop 对象 |

> **注意**：运行过程中，只能通过 `loop()->runInLoop()` 或 `loop()->run()` 向 LoopThread 注入任务，不能直接调用其他 Loop 方法。不可在外部 delete Loop 对象。

### Async — 异步操作

将阻塞性的系统调用转换为异步回调形式，利用线程池在后台线程执行，完成后回到主线程回调。

| 方法 | 说明 |
|------|------|
| `Async(thread_pool)` | 构造，指定线程池 |
| `readFile(filename, cb)` | 异步读取文件，cb 返回 (errcode, content) |
| `readFileLines(filename, cb)` | 异步读取文件行列表 |
| `writeFile(filename, content, sync, cb)` | 异步写文件 |
| `appendFile(filename, content, sync, cb)` | 异步追加文件 |
| `removeFile(filename, cb)` | 异步删除文件 |
| `executeCmd(cmd, cb)` | 异步执行命令，cb 返回 (errcode) |

### RequestPool — 请求池

请求池用于管理异步请求的上下文数据，自动处理超时回复。

```cpp
template <class T>
class RequestPool {
    //! 初始化
    bool initialize(check_interval, check_times);
    //! 设置超时回调
    void setTimeoutAction(action);
    //! 创建新请求，返回 Token
    Token newRequest(T *req_ctx = nullptr);
    //! 更新请求上下文
    bool updateRequest(token, T *req_ctx);
    //! 取走请求上下文（移除记录）
    T* removeRequest(token);
    //! 清理
    void cleanup();
};
```

## 使用示例

### 线程池基本用法

> 完整示例见 `examples/eventx/thread_pool/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/eventx/thread_pool.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

using namespace tbox;
using namespace tbox::event;

int main() {
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    eventx::ThreadPool tp(sp_loop);
    tp.initialize(2, 4);  //! 最少2个线程，最多4个

    //! 后台线程执行耗时操作，完成后主线程处理结果
    tp.execute(
        [] {  //! worker 线程：执行耗时计算
            LogInfo("computing in worker thread...");
            // ... 耗时操作 ...
        },
        [] {  //! 主线程：处理结果
            LogInfo("result received in main thread");
            // ... 使用计算结果 ...
        }
    );

    //! 5秒后退出
    sp_loop->exitLoop(std::chrono::seconds(5));
    sp_loop->runLoop();

    tp.cleanup();
    LogOutput_Disable();
    return 0;
}
```

### 定时池

> 完整示例见 `examples/eventx/timer_fd/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/eventx/timer_pool.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

using namespace tbox;
using namespace tbox::event;

int main() {
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    eventx::TimerPool timer_pool(sp_loop);

    //! 每秒执行一次
    auto token_every = timer_pool.doEvery(std::chrono::seconds(1),
        [] { LogInfo("periodic tick"); });

    //! 3秒后执行一次
    auto token_after = timer_pool.doAfter(std::chrono::seconds(3),
        [] { LogInfo("one-shot timeout"); });

    //! 5秒后取消所有定时器并退出
    timer_pool.doAfter(std::chrono::seconds(5),
        [&] {
            timer_pool.cancel(token_every);
            timer_pool.cancel(token_after);
            sp_loop->exitLoop();
        });

    sp_loop->runLoop();
    timer_pool.cleanup();

    LogOutput_Disable();
    return 0;
}
```

### 异步文件操作

```cpp
#include <tbox/eventx/async.h>

//! 在 main 模块中使用 Async
class App : public tbox::main::Module {
  public:
    App(Context &ctx) : Module("app", ctx), async_(ctx.thread_pool()) { }

    bool onStart() override {
        //! 异步读取文件，不阻塞主线程
        async_.readFile("/data/config.json",
            [](int errcode, std::string &content) {
                if (errcode == 0) {
                    LogInfo("file content: %s", content.c_str());
                } else {
                    LogErr("read file failed, errcode=%d", errcode);
                }
            });
        return true;
    }

  private:
    eventx::Async async_;
};
```

## 常见场景

1. **耗时计算**：将复杂计算放到 ThreadPool，完成后在主线程使用结果
2. **文件 I/O**：使用 Async 异步读写文件，不阻塞事件循环
3. **大量定时器**：使用 TimerPool 批量管理定时任务，无需手动管理 TimerEvent 生命期
4. **多 Loop 协作**：使用 LoopThread 在独立线程运行另一个事件循环
5. **请求超时管理**：使用 RequestPool 自动处理请求超时回复

## 注意事项

1. **ThreadPool 回调线程**：`main_cb` 回调在主线程（Loop 线程）中执行，`backend_task` 在 worker 线程中执行
2. **TimerPool 生命期倒挂**：如果定时器回调持有了短生命期对象的指针，该对象析构前必须 cancel 定时器
3. **LoopThread 限制**：运行过程中只能通过 `loop()->runInLoop()` 或 `loop()->run()` 与 LoopThread 交互
4. **ThreadPool cleanup**：调用 `cleanup()` 会等待所有 worker 线程结束，确保在程序退出前调用
5. **Async errcode**：errcode=0 表示成功，非0表示失败（如文件不存在、权限不足等）

## 相关模块

- **event**：提供基础的 Loop，是 eventx 的底层依赖
- **main**：框架自动创建 ThreadPool/TimerPool/Async，通过 Context 提供
- **base**：提供 Cabinet/Token/defines 等基础设施
