# Event-Driven Module (event)

## What is it?

The event module is the core foundation of cpp-tbox, providing an event loop (Loop) and three basic event types (FdEvent, TimerEvent, SignalEvent). It is the heart of the entire framework. Almost all other modules depend on the event module to run.

## Why do you need it?

In service-type programs, the program needs to respond to multiple events simultaneously: network data arrival, timed task expiration, signal interrupts, etc. If handled with traditional multi-threading, you encounter issues like complex thread synchronization and resource contention. The event-driven model uses a single-threaded event loop to efficiently handle all asynchronous events, avoiding thread switching overhead.

## Header Files

```cpp
#include <tbox/event/loop.h>          //! Event loop
#include <tbox/event/fd_event.h>      //! File descriptor event
#include <tbox/event/timer_event.h>   //! Timer event
#include <tbox/event/signal_event.h>  //! Signal event
#include <tbox/event/event.h>         //! Event base class
```

## Core Classes and Interfaces

### Loop — Event Loop

The event loop is the dispatch center for all event handling. The program enters the loop via `runLoop()`, monitoring and dispatching events within the loop.

| Method | Description |
|--------|-------------|
| `Loop::New()` | Create an event loop of the default type |
| `Loop::New(engine_type)` | Create an event loop with a specified engine type (e.g., "epoll", "poll") |
| `Loop::Engines()` | Get the list of available engines |
| `runLoop(Mode)` | Run the event loop; Mode::kOnce executes once, Mode::kForever runs continuously |
| `exitLoop(wait_time)` | Exit the event loop, optionally specifying a wait time |
| `isInLoopThread()` | Check whether the current thread is the Loop thread |
| `isRunning()` | Check whether the Loop is currently running |
| `newFdEvent(what)` | Create an FdEvent object |
| `newTimerEvent(what)` | Create a TimerEvent object |
| `newSignalEvent(what)` | Create a SignalEvent object |
| `cleanup()` | Clean up Loop resources |

#### Task Injection Methods

Loop provides three methods for injecting functions into the loop for execution. Their differences are as follows:

| Method | Characteristics | Suitable Scenarios |
|--------|-----------------|--------------------|
| `runInLoop(func)` | Uses locking; supports cross-thread and cross-Loop calls | Delegating tasks between different Loops, or other threads dispatching tasks to the Loop thread |
| `runNext(func)` | No locking; does not support cross-thread calls; only callable within the Loop thread | Execute immediately after the current callback completes, e.g., freeing the object itself |
| `run(func)` | Auto-selects: uses runNext within the Loop thread, otherwise uses runInLoop | When unsure which one to use, just pick this one |

> **Usage tip**: Use `runNext()` when you are certain you are in the Loop thread, use `runInLoop()` when you are certain you are not in the Loop thread, and use `run()` when you are unsure.

All injection methods return `RunId`, which can be used to cancel unexecuted tasks via `cancel(RunId)`.

### FdEvent — File Descriptor Event

Monitors readable, writable, and exception events on a file descriptor (fd).

| Method | Description |
|--------|-------------|
| `initialize(fd, events, mode)` | Initialize; events is a combination of kReadEvent/kWriteEvent/kExceptEvent |
| `setCallback(cb)` | Set callback function with parameter `short events` |
| `enable()` | Enable monitoring |
| `disable()` | Disable monitoring |

Event modes:
- `Mode::kPersist` — Persistent monitoring; does not auto-cancel after the event triggers
- `Mode::kOneshot` — One-time monitoring; auto-cancels after the event triggers

### TimerEvent — Timer Event

Triggers a callback after a specified time.

| Method | Description |
|--------|-------------|
| `initialize(time_span, mode)` | Initialize; time_span is the duration in milliseconds |
| `setCallback(cb)` | Set callback function |
| `enable()` | Enable the timer |
| `disable()` | Disable the timer |

### SignalEvent — Signal Event

Monitors Linux signals (e.g., SIGINT, SIGTERM).

| Method | Description |
|--------|-------------|
| `initialize(signum, mode)` | Initialize; monitor a single signal |
| `initialize(sigset, mode)` | Initialize; monitor a signal set |
| `initialize({sig1,sig2,...}, mode)` | Initialize; monitor a signal list |
| `setCallback(cb)` | Set callback function with parameter `int signum` |

## Usage Examples

### Timer Example

> Full example available at `examples/event/02_timer/`

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

    //! Create a periodic timer that triggers every second
    auto sp_timer = sp_loop->newTimerEvent("timer");
    SetScopeExitAction([sp_timer] { delete sp_timer; });

    sp_timer->initialize(std::chrono::seconds(1), Event::Mode::kPersist);
    sp_timer->setCallback([] { LogInfo("timer tick"); });
    sp_timer->enable();

    //! Exit after running for 5 seconds
    sp_loop->exitLoop(std::chrono::seconds(5));
    sp_loop->runLoop();

    LogOutput_Disable();
    return 0;
}
```

### Signal Handling Example

> Full example available at `examples/event/03_signal/`

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

    //! Monitor SIGINT and SIGTERM signals
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

### runInLoop Cross-Thread Task Injection

> Full example available at `examples/event/04_run_in_loop/`

```cpp
//! Inject a task into the Loop from another thread
std::thread t(
    [sp_loop] {
        //! Safe cross-thread call
        sp_loop->runInLoop([] { LogInfo("task from other thread"); });
    }
);
```

### runNext Safe Self-Release

> Full example available at `examples/event/07_delay_delete/`

```cpp
//! Safely release the object itself within a callback
void MyClass::onTimeout() {
    //! Cannot directly delete this, which would cause accessing a destructed object within the callback
    //! Use runNext to perform the release after the callback completes
    loop_->runNext([this] { delete this; });
}
```

## Common Scenarios

1. **Program main loop**: Create a Loop, run `runLoop(Mode::kForever)`, exit via `exitLoop()`
2. **Timed tasks**: Use TimerEvent to create periodic or one-time timers
3. **Signal handling**: Use SignalEvent to capture SIGINT/SIGTERM for graceful program exit
4. **Cross-thread communication**: Other threads inject tasks into the Loop thread via `runInLoop()`
5. **Safe object release**: Use `runNext()` to release objects after callbacks finish

## Important Notes

1. **Loop is single-threaded**: All event callbacks execute in the Loop thread; do not perform time-consuming operations in callbacks, as this will block the event loop
2. **runNext is limited to the Loop thread**: Calling `runNext()` across threads is prohibited; cross-thread calls must use `runInLoop()`
3. **Event object lifecycle**: Event objects created through Loop (newFdEvent/newTimerEvent/newSignalEvent) need to be manually deleted; it is recommended to use `SetScopeExitAction` to manage their lifecycle
4. **Oneshot mode**: TimerEvent's kOneshot mode auto-disables after triggering; if you need to trigger again, you must re-enable it
5. **cancel() timing**: Tasks that have already started executing cannot be canceled; only unexecuted tasks can be canceled

## Related Modules

- **eventx**: Provides advanced features like thread pools and timer pools based on event
- **base**: Provides infrastructure such as log macros and ScopeExit
- **network**: Implements TCP/UDP/UART communication based on event's FdEvent
- **alarm**: Implements timed alarms based on event's TimerEvent
- **main**: The framework's built-in Loop object, accessible via Context

## Reference Image

![tbox-loop](../images/0001-tbox-loop.jpg)
