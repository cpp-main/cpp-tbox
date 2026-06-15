# 协程模块 (coroutine)

## 是什么？

coroutine 模块是基于 event 架构的协程库，帮助开发者用顺序型代码处理异步逻辑，避免事件驱动编程中的回调地狱和复杂状态机。

## 为什么需要它？

基于事件驱动的程序擅长处理 "当发生xx事件，就做yy动作" 的逻辑。如果事件之间相互孤立，很好处理。但一旦遇到顺序型业务逻辑，如"先做A，然后做B，如果A或B失败则做C..."，事件驱动模型需要设计复杂的状态机，代码零散难维护。

协程的优点：
- **轻量**：只需为每个协程分配一个栈，栈大小可指定（默认 8KB）
- **可控切换**：协程之间切换由程序自行控制，yield()/wait() 主动切换
- **无资源抢占**：不需要锁等同步机制

相比线程：
- 线程较重，占 CPU 且耗内存
- 线程切换不可控
- 资源抢占不易管理

## 头文件

```cpp
#include <tbox/coroutine/scheduler.h>    //! 协程调度器
#include <tbox/coroutine/channel.hpp>    //! 通道（类似 Golang chan）
#include <tbox/coroutine/mutex.hpp>      //! 互斥量
#include <tbox/coroutine/semaphore.hpp>  //! 信号量
#include <tbox/coroutine/condition.hpp>  //! 条件量
#include <tbox/coroutine/broadcast.hpp>  //! 广播
```

## 核心类与接口

### Scheduler — 协程调度器

| 方法 | 说明 |
|------|------|
| `Scheduler(loop)` | 构造，指定事件循环 |
| `create(entry, run_now, name, stack_size)` | 创建协程，返回 RoutineToken |
| `resume(token)` | 恢复指定协程 |
| `cancel(token)` | 取消协程（发送取消请求，非立即停止） |
| `wait()` | 切换到主协程，等待被 resume 唤醒 |
| `yield()` | 切换到主协程，下一个事件循环继续执行 |
| `join(other)` | 一个协程等待另一个协程结束 |
| `getToken()` | 获取当前协程 Token |
| `isCanceled()` | 当前协程是否被取消 |
| `getName()` | 当前协程名称 |
| `getLoop()` | 获取事件循环 |
| `cleanup()` | 强行停止并清理所有协程 |

### Channel<T> — 通道

类似 Golang 的 chan，协程间传递数据：

```cpp
Channel<int> ch(sch);

//! 发送端
ch << 42;

//! 接收端
int value;
ch >> value;  //! 如果队列空则等待
```

### Mutex — 互斥量

协程间互斥访问：

```cpp
Mutex mtx(sch);

//! 推荐使用 Locker 自动管理
{
    Mutex::Locker locker(mtx);  //! 自动 lock
    //! ... 临界区操作 ...
}   //! 自动 unlock
```

### Semaphore — 信号量

```cpp
Semaphore sem(sch, 3);  //! 初始计数为3

sem.acquire();  //! 请求资源（计数为0时等待）
sem.release();  //! 释放资源
```

### Condition<T> — 条件量

等待多个条件同时满足或任一满足：

```cpp
Condition<std::string> cond(sch, Condition<std::string>::Logic::kAll);
cond.add("event_a");
cond.add("event_b");

//! 协程A等待
cond.wait();  //! 等待 event_a 和 event_b 都发生

//! 协程B发出信号
cond.post("event_a");
//! 协程C发出信号
cond.post("event_b");  //! 两个条件都满足，唤醒协程A
```

### Broadcast — 广播

多个协程等待一个信号，信号发出时所有等待协程都被唤醒：

```cpp
Broadcast bc(sch);

//! 多个协程等待
bc.wait();

//! 发出广播
bc.post();  //! 所有等待的协程被唤醒
```

## 使用示例

### 基础用法

> 完整示例见模块 README 和单元测试用例

```cpp
#include <tbox/event/loop.h>
#include <tbox/coroutine/scheduler.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::coroutine;

int main() {
    LogOutput_Enable();

    Loop *sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    Scheduler sch(sp_loop);

    //! 定义协程1
    int routine1_count = 0;
    sch.create(
        [&] (Scheduler &sch) {
            for (int i = 0; i < 20; ++i) {
                ++routine1_count;
                sch.yield();  //! 主动让出执行权
            }
        }, true, "routine1"
    );

    //! 定义协程2
    int routine2_count = 0;
    sch.create(
        [&] (Scheduler &sch) {
            for (int i = 0; i < 10; ++i) {
                ++routine2_count;
                sch.yield();
            }
        }, true, "routine2"
    );

    sp_loop->exitLoop(std::chrono::seconds(1));
    sp_loop->runLoop();

    LogInfo("r1=%d, r2=%d", routine1_count, routine2_count);

    LogOutput_Disable();
    return 0;
}
```

### 协程间通信 — Channel

```cpp
Scheduler sch(sp_loop);

Channel<int> ch(sch);

//! 生产者协程
sch.create([&] (Scheduler &sch) {
    for (int i = 0; i < 5; ++i) {
        ch << i;
        sch.yield();
    }
});

//! 消费者协程
sch.create([&] (Scheduler &sch) {
    int value;
    while (ch >> value) {
        LogInfo("received: %d", value);
    }
});
```

### 协程间互斥 — Mutex

```cpp
Mutex mtx(sch);

sch.create([&] (Scheduler &sch) {
    Mutex::Locker locker(mtx);  //! 自动加锁
    LogInfo("locked, doing work");
    sch.yield();
    //! ... 临界区操作 ...
});  //! locker 析构时自动解锁
```

### 等待协程结束 — join

```cpp
auto other_token = sch.create([&] (Scheduler &sch) {
    //! 另一个协程的工作
    sch.yield();
    sch.yield();
});

sch.create([&] (Scheduler &sch) {
    sch.join(other_token);  //! 等待 other_token 协程结束
    LogInfo("other routine finished");
});
```

## 常见场景

1. **顺序型业务逻辑**：将多步骤异步流程用协程写为顺序代码
2. **生产者-消费者**：使用 Channel 在协程间传递数据
3. **共享资源保护**：使用 Mutex 保护协程间的临界区
4. **等待条件满足**：使用 Condition 等待多个条件
5. **广播通知**：使用 Broadcast 同时唤醒多个协程

## 注意事项

1. **yield vs wait**：`yield()` 切换到主协程，下一个事件循环自动继续；`wait()` 切换到主协程，需要被 `resume()` 唤醒才能继续
2. **协程是单线程的**：所有协程在同一个 Loop 线程中调度，不存在真正的并发，不需要原子操作
3. **栈大小**：默认栈大小 8KB（`ROUTINE_STACK_DEFAULT_SIZE`），可通过 create() 参数指定更大的栈
4. **cancel 是异步的**：cancel() 只是发送取消请求，协程在下次 wait/yield 时检查 isCanceled() 并退出
5. **Channel 的 >> 返回值**：当协程被 cancel 时，`>>` 操作返回 false
6. **Condition 不支持多协程同时等**：同一 Condition 只能有一个协程在 wait()

## 相关模块

- **event**：协程基于 Loop 调度运行
- **main**：框架自动创建 Scheduler，通过 `ctx.coroutine()` 获取
- **base**：提供 Cabinet/Token 等基础设施
