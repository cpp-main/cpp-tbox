# Event Extension Module (eventx)

## What is it?

The eventx module provides advanced asynchronous programming components built on top of the event module: ThreadPool, TimerPool, LoopThread, Async, TimeoutMonitor, and RequestPool. These components make it easier for developers to handle complex scenarios such as multi-thread coordination, timer task management, and request timeouts.

## Why do you need it?

The event module provides a single-threaded event loop, but in practice you often need:
- Move time-consuming computation or I/O operations to background threads, then return to the main thread to process results
- Create a large number of timers without having to manage each TimerEvent's lifecycle individually
- Run another event loop in a separate thread
- Convert blocking system calls (such as file read/write) into asynchronous callback form

eventx is designed to solve these problems.

## Header Files

```cpp
#include <tbox/eventx/thread_pool.h>       //! ThreadPool
#include <tbox/eventx/timer_pool.h>        //! TimerPool
#include <tbox/eventx/loop_thread.h>       //! LoopThread
#include <tbox/eventx/async.h>             //! Async
#include <tbox/eventx/request_pool.hpp>     //! RequestPool
#include <tbox/eventx/timeout-monitor.hpp> //! TimeoutMonitor
#include <tbox/eventx/thread_executor.h>   //! ThreadExecutor interface
```

## Core Classes and Interfaces

### ThreadPool

ThreadPool is used to delegate time-consuming tasks to background threads for execution, and then return to the main thread to execute callbacks upon completion.

| Method | Description |
|--------|-------------|
| `ThreadPool(main_loop)` | Constructor, specify the main thread's Loop |
| `initialize(min, max)` | Initialize, specify the number of resident threads and maximum threads |
| `execute(task, prio)` | Execute task in a worker thread, prio is priority [-2,2] |
| `execute(task, main_cb, prio)` | Worker thread executes task, then main thread executes main_cb upon completion |
| `execute(task)` | Execute task in a worker thread (no callback) |
| `execute(task, main_cb)` | Worker thread executes task, then main thread executes main_cb upon completion |
| `getTaskStatus(token)` | Get task status |
| `cancel(token)` | Cancel a task |
| `cleanup()` | Clean up resources, wait for all worker threads to finish |
| `snapshot()` | Get thread pool snapshot (thread count, idle count, task count, etc.) |

### TimerPool

TimerPool allows developers to easily create timed tasks without worrying about TimerEvent lifecycle management.

| Method | Description |
|--------|-------------|
| `TimerPool(loop)` | Constructor, specify the Loop |
| `doEvery(msec, cb)` | Periodic timed task, returns TimerToken |
| `doAfter(msec, cb)` | One-shot delayed task, returns TimerToken |
| `doAt(time_point, cb)` | Execute at a specified time point, returns TimerToken |
| `cancel(token)` | Cancel a timed task |
| `cleanup()` | Clean up all timers |

> **Note**: When using `cancel()` to cancel a timer, ensure that objects held by the callback function are still alive, to avoid lifetime inversion issues.

### LoopThread

Runs an event loop in a separate thread, commonly used in scenarios where events need to be processed in another thread.

| Method | Description |
|--------|-------------|
| `LoopThread(run_now, name)` | Constructor, specify whether to run immediately and the Loop name |
| `start()` | Start the thread |
| `stop()` | Stop the thread |
| `isRunning()` | Whether the thread is running |
| `loop()` | Return the Loop object |

> **Note**: During runtime, you can only inject tasks into LoopThread via `loop()->runInLoop()` or `loop()->run()`. Do not call other Loop methods directly. Do not delete the Loop object externally.

### Async

Converts blocking system calls into asynchronous callback form, utilizing a thread pool to execute in a background thread, then returning to the main thread for callback upon completion.

| Method | Description |
|--------|-------------|
| `Async(thread_pool)` | Constructor, specify the thread pool |
| `readFile(filename, cb)` | Asynchronously read a file, cb returns (errcode, content) |
| `readFileLines(filename, cb)` | Asynchronously read file lines list |
| `writeFile(filename, content, sync, cb)` | Asynchronously write a file |
| `appendFile(filename, content, sync, cb)` | Asynchronously append to a file |
| `removeFile(filename, cb)` | Asynchronously delete a file |
| `executeCmd(cmd, cb)` | Asynchronously execute a command, cb returns (errcode) |

### RequestPool

RequestPool is used to manage context data for asynchronous requests, automatically handling timeout responses.

```cpp
template <class T>
class RequestPool {
    //! Initialize
    bool initialize(check_interval, check_times);
    //! Set timeout callback
    void setTimeoutAction(action);
    //! Create a new request, return Token
    Token newRequest(T *req_ctx = nullptr);
    //! Update request context
    bool updateRequest(token, T *req_ctx);
    //! Take away request context (remove record)
    T* removeRequest(token);
    //! Cleanup
    void cleanup();
};
```

## Usage Examples

### ThreadPool Basic Usage

> Full example see `examples/eventx/thread_pool/`

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
    tp.initialize(2, 4);  //! Minimum 2 threads, maximum 4

    //! Background thread executes time-consuming operation, then main thread processes result
    tp.execute(
        [] {  //! Worker thread: execute time-consuming computation
            LogInfo("computing in worker thread...");
            // ... time-consuming operation ...
        },
        [] {  //! Main thread: process result
            LogInfo("result received in main thread");
            // ... use computation result ...
        }
    );

    //! Exit after 5 seconds
    sp_loop->exitLoop(std::chrono::seconds(5));
    sp_loop->runLoop();

    tp.cleanup();
    LogOutput_Disable();
    return 0;
}
```

### TimerPool

> Full example see `examples/eventx/timer_fd/`

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

    //! Execute every second
    auto token_every = timer_pool.doEvery(std::chrono::seconds(1),
        [] { LogInfo("periodic tick"); });

    //! Execute once after 3 seconds
    auto token_after = timer_pool.doAfter(std::chrono::seconds(3),
        [] { LogInfo("one-shot timeout"); });

    //! Cancel all timers and exit after 5 seconds
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

### Asynchronous File Operations

```cpp
#include <tbox/eventx/async.h>

//! Using Async in the main module
class App : public tbox::main::Module {
  public:
    App(Context &ctx) : Module("app", ctx), async_(ctx.thread_pool()) { }

    bool onStart() override {
        //! Asynchronously read file without blocking the main thread
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

## Common Scenarios

1. **Time-consuming computation**: Put complex computation into ThreadPool, then use the result in the main thread upon completion
2. **File I/O**: Use Async for asynchronous file read/write without blocking the event loop
3. **Large number of timers**: Use TimerPool to manage timed tasks in bulk without manually managing TimerEvent lifetimes
4. **Multi-Loop coordination**: Use LoopThread to run another event loop in a separate thread
5. **Request timeout management**: Use RequestPool to automatically handle request timeout responses

## Important Notes

1. **ThreadPool callback threading**: The `main_cb` callback executes in the main thread (Loop thread), and `backend_task` executes in a worker thread
2. **TimerPool lifetime inversion**: If a timer callback holds a pointer to a short-lifetime object, the timer must be canceled before that object is destructed
3. **LoopThread restrictions**: During runtime, you can only interact with LoopThread via `loop()->runInLoop()` or `loop()->run()`
4. **ThreadPool cleanup**: Calling `cleanup()` will wait for all worker threads to finish, ensure it is called before program exit
5. **Async errcode**: errcode=0 indicates success, non-zero indicates failure (such as file not found, insufficient permissions, etc.)

## Related Modules

- **event**: Provides the base Loop, which is the underlying dependency of eventx
- **main**: The framework automatically creates ThreadPool/TimerPool/Async and provides them through Context
- **base**: Provides infrastructure such as Cabinet/Token/defines
