# Coroutine Module (coroutine)

## What is it?

The coroutine module is a coroutine library built on the event architecture, helping developers handle asynchronous logic with sequential-style code, avoiding callback hell and complex state machines inherent in event-driven programming.

## Why do you need it?

Event-driven programs excel at handling "when event X occurs, perform action Y" logic. If events are independent of each other, this is easy to manage. But when dealing with sequential business logic, such as "first do A, then do B, and if either A or B fails, do C...", the event-driven model requires designing complex state machines, resulting in scattered and hard-to-maintain code.

Advantages of coroutines:
- **Lightweight**: Only requires allocating a stack for each coroutine, with a configurable stack size (default 8KB)
- **Controllable switching**: Coroutine switching is controlled by the program itself, via explicit yield()/wait() calls
- **No resource preemption**: No need for locks or other synchronization mechanisms

Compared to threads:
- Threads are heavier, consuming both CPU and memory
- Thread switching is uncontrollable
- Resource preemption is difficult to manage

## Header Files

```cpp
#include <tbox/coroutine/scheduler.h>    //! Coroutine scheduler
#include <tbox/coroutine/channel.hpp>    //! Channel (similar to Golang chan)
#include <tbox/coroutine/mutex.hpp>      //! Mutex
#include <tbox/coroutine/semaphore.hpp>  //! Semaphore
#include <tbox/coroutine/condition.hpp>  //! Condition
#include <tbox/coroutine/broadcast.hpp>  //! Broadcast
```

## Core Classes and Interfaces

### Scheduler — Coroutine Scheduler

| Method | Description |
|------|------|
| `Scheduler(loop)` | Constructor, specifies the event loop |
| `create(entry, run_now, name, stack_size)` | Creates a coroutine, returns RoutineToken |
| `resume(token)` | Resumes the specified coroutine |
| `cancel(token)` | Cancels a coroutine (sends a cancel request, not an immediate stop) |
| `wait()` | Switches to the main coroutine, waits to be woken up by resume |
| `yield()` | Switches to the main coroutine, continues execution in the next event loop iteration |
| `join(other)` | One coroutine waits for another coroutine to finish |
| `getToken()` | Gets the current coroutine Token |
| `isCanceled()` | Whether the current coroutine has been canceled |
| `getName()` | Gets the current coroutine name |
| `getLoop()` | Gets the event loop |
| `cleanup()` | Forcefully stops and cleans up all coroutines |

### Channel<T> — Channel

Similar to Golang's chan, used for passing data between coroutines:

```cpp
Channel<int> ch(sch);

//! Sender
ch << 42;

//! Receiver
int value;
ch >> value;  //! Waits if the queue is empty
```

### Mutex — Mutex

Mutual exclusion between coroutines:

```cpp
Mutex mtx(sch);

//! Recommended: use Locker for automatic management
{
    Mutex::Locker locker(mtx);  //! Auto lock
    //! ... critical section operations ...
}   //! Auto unlock
```

### Semaphore — Semaphore

```cpp
Semaphore sem(sch, 3);  //! Initial count of 3

sem.acquire();  //! Request a resource (waits when count is 0)
sem.release();  //! Release a resource
```

### Condition<T> — Condition

Wait for multiple conditions to be all satisfied or any one satisfied:

```cpp
Condition<std::string> cond(sch, Condition<std::string>::Logic::kAll);
cond.add("event_a");
cond.add("event_b");

//! Coroutine A waits
cond.wait();  //! Waits for both event_a and event_b to occur

//! Coroutine B signals
cond.post("event_a");
//! Coroutine C signals
cond.post("event_b");  //! Both conditions satisfied, wakes up Coroutine A
```

### Broadcast — Broadcast

Multiple coroutines wait for a single signal; when the signal is posted, all waiting coroutines are woken up:

```cpp
Broadcast bc(sch);

//! Multiple coroutines wait
bc.wait();

//! Post broadcast
bc.post();  //! All waiting coroutines are woken up
```

## Usage Examples

### Basic Usage

> See the module README and unit test cases for complete examples

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

    //! Define coroutine 1
    int routine1_count = 0;
    sch.create(
        [&] (Scheduler &sch) {
            for (int i = 0; i < 20; ++i) {
                ++routine1_count;
                sch.yield();  //! Voluntarily yield execution
            }
        }, true, "routine1"
    );

    //! Define coroutine 2
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

### Inter-Coroutine Communication — Channel

```cpp
Scheduler sch(sp_loop);

Channel<int> ch(sch);

//! Producer coroutine
sch.create([&] (Scheduler &sch) {
    for (int i = 0; i < 5; ++i) {
        ch << i;
        sch.yield();
    }
});

//! Consumer coroutine
sch.create([&] (Scheduler &sch) {
    int value;
    while (ch >> value) {
        LogInfo("received: %d", value);
    }
});
```

### Inter-Coroutine Mutual Exclusion — Mutex

```cpp
Mutex mtx(sch);

sch.create([&] (Scheduler &sch) {
    Mutex::Locker locker(mtx);  //! Auto lock
    LogInfo("locked, doing work");
    sch.yield();
    //! ... critical section operations ...
});  //! Locker destructor auto unlocks
```

### Waiting for a Coroutine to Finish — join

```cpp
auto other_token = sch.create([&] (Scheduler &sch) {
    //! Work of another coroutine
    sch.yield();
    sch.yield();
});

sch.create([&] (Scheduler &sch) {
    sch.join(other_token);  //! Wait for the other_token coroutine to finish
    LogInfo("other routine finished");
});
```

## Common Scenarios

1. **Sequential business logic**: Write multi-step asynchronous flows as sequential code using coroutines
2. **Producer-Consumer**: Use Channel to pass data between coroutines
3. **Shared resource protection**: Use Mutex to protect critical sections between coroutines
4. **Waiting for conditions**: Use Condition to wait for multiple conditions
5. **Broadcast notification**: Use Broadcast to wake up multiple coroutines simultaneously

## Important Notes

1. **yield vs wait**: `yield()` switches to the main coroutine and automatically continues in the next event loop iteration; `wait()` switches to the main coroutine and requires `resume()` to be called before it can continue
2. **Coroutines are single-threaded**: All coroutines are scheduled within the same Loop thread; there is no real concurrency, so atomic operations are not needed
3. **Stack size**: The default stack size is 8KB (`ROUTINE_STACK_DEFAULT_SIZE`); a larger stack can be specified via the create() parameter
4. **cancel is asynchronous**: cancel() only sends a cancel request; the coroutine checks isCanceled() at the next wait/yield and exits
5. **Channel >> return value**: When a coroutine is canceled, the `>>` operation returns false
6. **Condition does not support multiple coroutines waiting simultaneously**: Only one coroutine can wait() on the same Condition at a time

## Related Modules

- **event**: Coroutines are scheduled and run based on Loop
- **main**: The framework automatically creates a Scheduler, accessible via `ctx.coroutine()`
- **base**: Provides infrastructure such as Cabinet/Token
