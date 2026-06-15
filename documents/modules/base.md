# Base Module (base)

## What is it?

The base module is the lowest-level dependency module of cpp-tbox, providing infrastructure such as log macros, assertions, object pools, lifetime tags, scope exit actions, cabinets, common definitions, and more. All other modules depend on the base module.

## Why do you need it?

In C++ project development, logging, resource management, and assertion checking are the most fundamental needs. The base module uniformly encapsulates these common functionalities so that all modules share consistent log output interfaces and resource management patterns, avoiding redundant implementations.

## Header Files

```cpp
#include <tbox/base/log.h>              //! Log macros
#include <tbox/base/log_impl.h>         //! Log implementation details
#include <tbox/base/log_output.h>       //! Log output switch
#include <tbox/base/assert.h>           //! Assertion macros
#include <tbox/base/defines.h>          //! Common macro definitions (NONCOPYABLE, etc.)
#include <tbox/base/scope_exit.hpp>     //! Scope exit action
#include <tbox/base/object_pool.hpp>    //! Object pool
#include <tbox/base/lifetime_tag.hpp>   //! Lifetime tag
#include <tbox/base/cabinet.hpp>        //! Cabinet
#include <tbox/base/cabinet_token.h>    //! Cabinet token
#include <tbox/base/catch_throw.h>      //! Exception catch
#include <tbox/base/memblock.h>         //! Memory block
#include <tbox/base/recorder.h>         //! Recorder
#include <tbox/base/backtrace.h>        //! Call stack backtrace
#include <tbox/base/json.hpp>           //! JSON library (nlohmann/json)
#include <tbox/base/json_fwd.h>         //! JSON forward declaration
#include <tbox/base/version.h>          //! Version info
```

## Core Classes and Interfaces

### Log Macros (log.h)

Logging is the most commonly used feature in a project. The base module defines 8 log levels and corresponding print macros:

| Log Level | Macro | Description |
|---------|------|------|
| FATAL (0) | `LogFatal(fmt, ...)` | Program will crash |
| ERROR (1) | `LogErr(fmt, ...)` | Severe problem that the program cannot handle |
| WARN (2) | `LogWarn(fmt, ...)` | Internal anomaly, but the program can handle it |
| NOTICE (3) | `LogNotice(fmt, ...)` | Not very severe but should be noted, such as invalid input |
| IMPORTANT (4) | `LogImportant(fmt, ...)` | Important message |
| INFO (5) | `LogInfo(fmt, ...)` | Normal message |
| DEBUG (6) | `LogDbg(fmt, ...)` | Internal program debug information |
| TRACE (7) | `LogTrace(fmt, ...)` | Temporary debug log |

Helper macros:

| Macro | Description |
|------|------|
| `LogTag()` | Prints "==> Run Here <==", marking code execution location |
| `LogUndo()` | Prints "!!! Undo !!!", marking unimplemented functionality |
| `LogErrno(err, fmt, ...)` | Prints errno error code and its meaning |

> **Note**: `LogDbg` and `LogTrace` can be disabled at compile time via the `STATIC_LOG_LEVEL` compile option, reducing log volume in production environments.

#### MODULE_ID Definition

Log output includes a module identifier. You need to define `MODULE_ID` in compile options, e.g. `-DMODULE_ID=alarm`. If not defined, the module name in logs will display as "???".

#### Log Output Switch (log_output.h)

```cpp
LogOutput_Enable();   //! Enable log output to stdout
LogOutput_Disable();  //! Disable log output
```

> `LogOutput_Enable()` is the simplest way to output logs, printing them to stdout. For richer log configuration, please use the **log module**.

### Assertion Macros (assert.h)

```cpp
TBOX_ASSERT(expr);  //! In debug mode, if the condition is false, prints LogFatal and abort()
```

- In `NDEBUG` mode (Release build), `TBOX_ASSERT` does nothing
- In debug mode, a failed assertion prints an error message and terminates the program

### Scope Exit Action (scope_exit.hpp)

The `SetScopeExitAction` macro automatically executes a specified action when a code block exits, similar to Go's defer.

```cpp
SetScopeExitAction(action);  //! Execute action when the current scope exits
```

**Typical usage**: managing the release of dynamically allocated resources.

```cpp
Loop* sp_loop = Loop::New();
SetScopeExitAction([sp_loop] { delete sp_loop; });
//! ... use sp_loop ...
//! sp_loop is automatically deleted when the function exits
```

> **Note**: The `ScopeExitActionGuard` object created by `SetScopeExitAction` is non-copyable and non-movable (NONCOPYABLE/IMMOVABLE). Execution can be canceled via `cancel()`.

### Object Pool (object_pool.hpp)

`ObjectPool<T>` is a template class used to reduce the performance overhead of frequent new/delete operations on objects. It caches freed memory blocks through a free block list, avoiding repeated allocation and deallocation of memory.

```cpp
//! Create an object pool
ObjectPool<MyStruct> op;
//! Or specify the number of free blocks to retain
ObjectPool<MyStruct> op(64);

//! Allocate an object (equivalent to new, but faster)
auto p1 = op.alloc(1, "hello");  //! Supports constructor arguments

//! Free an object (equivalent to delete)
op.free(p1);

//! Get statistics
auto stat = op.getStat();
//! stat.total_alloc_times  — Total allocation count
//! stat.total_free_times   — Total free count
//! stat.peak_alloc_number  — Maximum simultaneous allocations
//! stat.peak_free_number   — Maximum free cache count
```

> **Important**: Objects allocated via ObjectPool **must be freed using ObjectPool**, not with `delete`.

### Lifetime Tag (lifetime_tag.hpp)

`LifetimeTag` is used to mark whether an object's lifetime is valid, solving the risk of a pointer referencing an object that has been prematurely destructed.

```cpp
struct HostObject {
    int value = 0;
    LifetimeTag tag;    //! Lifetime tag
};

HostObject *o = new HostObject;
LifetimeTag::Watcher w = o->tag;  //! Create a watcher

if (w)    //! true, object is alive
    cout << "value:" << o->value << endl;

delete o;  //! Destruct the object

if (w)    //! false, object has been destructed
    cout << "Cannot safely access" << endl;
```

> **Note**: LifetimeTag currently does not have lock protection and does not support multithreading.

### Cabinet (cabinet.hpp)

`Cabinet<T>` is a secure object container with token-based access. Objects are accessed via a Token; even if an object is deleted, an old Token will not mistakenly retrieve a new object.

```cpp
Cabinet<MyClass> cab;

//! Store an object, get a Token
auto token = cab.alloc(new MyClass);

//! Remove an object
auto p = cab.free(token);  //! Removes the record and returns the object pointer

//! Check if a Token is valid
auto p2 = cab.at(token);   //! Does not remove, only looks up
```

### Common Definitions (defines.h)

| Macro | Description |
|------|------|
| `NONCOPYABLE(classname)` | Disable copy constructor and assignment |
| `IMMOVABLE(classname)` | Disable move constructor and assignment |
| `DECLARE_COPY_FUNC(classname)` | Declare copy function (for Variables) |
| `CHECK_DELETE_RESET_OBJ(ptr)` | Delete pointer and set to nullptr |

## Usage Examples

### Printing Logs

> Full example at `examples/base/print_log/`

```cpp
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

#define MODULE_ID "my_app"

int main() {
    LogOutput_Enable();

    LogInfo("program started");
    LogDbg("debug info: count=%d", 42);
    LogWarn("unexpected input: %s", "abc");
    LogErr("file open failed");

    LogOutput_Disable();
    return 0;
}
```

### Assertion Checking

> Full example at `examples/base/assert/`

```cpp
#include <tbox/base/assert.h>
#include <tbox/base/log_output.h>

int main() {
    LogOutput_Enable();

    int value = 10;
    TBOX_ASSERT(value > 0);   //! In debug mode, if the condition holds, execution continues
    //! TBOX_ASSERT(value < 0); //! If the condition fails, prints LogFatal and abort

    LogOutput_Disable();
    return 0;
}
```

### Object Pool

> Full example at `examples/base/object_pool/`

```cpp
#include <tbox/base/object_pool.hpp>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

class MyStruct {
  public:
    MyStruct(int i, const std::string &s) : i_(i), s_(s) { }
    void print() { LogInfo("i:%d, s:%s", i_, s_.c_str()); }
  private:
    int i_;
    std::string s_;
};

int main() {
    LogOutput_Enable();

    ObjectPool<MyStruct> op;

    auto p1 = op.alloc(1, "hello");  //! Equivalent to new MyStruct(1, "hello")
    p1->print();

    op.free(p1);  //! Equivalent to delete p1, but the memory block is cached

    //! Re-allocating reuses the previously cached memory block, avoiding malloc
    auto p2 = op.alloc(2, "world");
    p2->print();
    op.free(p2);

    auto stat = op.getStat();
    LogInfo("alloc:%zu, free:%zu, peak:%zu",
            stat.total_alloc_times, stat.total_free_times, stat.peak_alloc_number);

    LogOutput_Disable();
    return 0;
}
```

### Lifetime Tag

> Full example at `examples/base/lifetime_tag/`

```cpp
#include <tbox/base/lifetime_tag.hpp>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

struct Resource {
    int data = 100;
    tbox::LifetimeTag tag;
};

int main() {
    LogOutput_Enable();

    Resource *res = new Resource;
    tbox::LifetimeTag::Watcher watcher = res->tag;

    LogInfo("alive: %d, data: %d", (bool)watcher, res->data);

    delete res;  //! Object destructed

    LogInfo("alive: %d", (bool)watcher);  //! watcher is false
    //! Cannot safely access res->data anymore

    LogOutput_Disable();
    return 0;
}
```

## Common Scenarios

1. **Logging**: All modules uniformly use `LogInfo/LogErr/LogDbg` and other macros to print logs
2. **Automatic resource release**: Use `SetScopeExitAction` to automatically delete new'd objects when a function exits
3. **High-frequency object allocation**: Use `ObjectPool` to reduce the performance overhead of frequent new/delete
4. **Pointer safety checking**: Use `LifetimeTag` + `Watcher` to check whether an object is still alive
5. **Disable copy/move**: Use `NONCOPYABLE/IMMOVABLE` macros to protect class semantic integrity

## Important Notes

1. **MODULE_ID must be defined**: If not defined, the module name in logs displays as "???", affecting log identification
2. **ObjectPool free vs delete**: Objects allocated via ObjectPool must only be freed using ObjectPool's `free()`, not with `delete`
3. **LifetimeTag does not support multithreading**: Currently there is no lock protection; it is only suitable for single-threaded or Loop-thread contexts
4. **SetScopeExitAction cannot cross scopes**: Its execution timing depends on the exit of the enclosing code block; be mindful of the lifetime of objects captured by lambdas
5. **TBOX_ASSERT is inactive in Release**: Assertions are ignored under NDEBUG compilation; do not use assertions as a substitute for error handling

## Related Modules

- **log**: Log channel implementation based on base/log.h, providing file/stdout/syslog and other output methods
- **event**: Depends on base's Cabinet, ObjectPool, defines, etc.
- **All modules**: base is the foundational dependency of all modules
