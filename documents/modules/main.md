# Application Framework Module (main)

## What is it?

The main module is the application startup framework. It provides a unified and complete encapsulation of the program startup process, allowing developers to focus only on business logic without worrying about startup procedures. It automatically creates common components such as the event loop, thread pool, timer pool, and coroutine scheduler, and provides them to business modules through the Context object.

## Why do you need it?

When developing service-type programs, you typically need to repeatedly write the following procedures: creating an event loop, initializing logging, configuring a thread pool, handling command-line arguments, responding to exit signals, etc. The main module encapsulates all these procedures uniformly, so developers only need to implement four steps for their business modules: initialize, start, stop, and cleanup.

![main-framework](../images/0008-main-framework.png)

## Header Files

```cpp
#include <tbox/main/main.h>       //! Main entry function and registration interface
#include <tbox/main/module.h>     //! Module base class
#include <tbox/main/context.h>    //! Process context
#include <tbox/main/args_parser.h> //! Command-line argument parser
#include <tbox/main/log.h>        //! Logging related
#include <tbox/main/trace.h>      //! Tracing related
```

## Core Classes and Interfaces

### Main / Start / Stop — Start and Stop

| Function | Description |
|----------|-------------|
| `Main(argc, argv)` | Run the tbox::main framework in the foreground, blocking until a stop signal is received |
| `Start(argc, argv)` | Run the tbox::main framework in the backend, non-blocking |
| `Stop()` | Stop the backend-running tbox::main framework |
| `RaiseStopSignal()` | Send a stop request to itself |

### Module — Business Module Base Class

The Module lifecycle follows this process:

```
Construct → Initialize → Start → .Running. → Stop → Cleanup → Destruct
```

Using setting up a computer as an analogy:
1. **Construct** — Place the devices one by one
2. **Initialize** — Plug in power, connect cables
3. **Start** — Turn on each device
4. ... Normal operation ...
5. **Stop** — Turn off each device
6. **Cleanup** — Disconnect cables
7. **Destruct** — Remove the devices one by one

| Method | Description |
|--------|-------------|
| `Module(name, ctx)` | Constructor, name is the module name, ctx is the process context |
| `add(child, required)` | Add a child module. When required=true, failure of the child module's initialization/start will cause the entire program startup to fail |
| `addAs(child, name, required)` | Add a child module and rename it |
| `name()` | Get the module name |
| `ctx()` | Get the process context |
| `state()` | Get the module state (kNone/kInited/kRunning) |

Virtual functions to override:

| Virtual Function | Description |
|------------------|-------------|
| `onFillDefaultConfig(Json)` | Fill default configuration parameters (Note: the logging system is not available at this stage) |
| `onInit(const Json &cfg)` | Initialize, read configuration, establish object connections |
| `onStart()` | Start the module, make objects begin working |
| `onStop()` | Stop the module, the reverse operation of onStart() |
| `onCleanup()` | Cleanup the module, the reverse operation of onInit() |

### Context — Process Context

Context provides the common components created by the framework, which business modules access via `ctx()`:

| Interface | Description |
|-----------|-------------|
| `ctx.loop()` | Event loop object |
| `ctx.thread_pool()` | Thread pool object |
| `ctx.timer_pool()` | Timer pool object |
| `ctx.async()` | Async operation object |
| `ctx.terminal()` | Interactive terminal object |
| `ctx.coroutine()` | Coroutine scheduler object |
| `ctx.running_time()` | Program running duration |
| `ctx.start_time_point()` | Program start time point |
| `ctx.args()` | Command-line argument list |

### Required Functions

Developers must implement the following functions for the framework to call:

| Function | Description |
|----------|-------------|
| `RegisterApps(Module &apps, Context &ctx)` | Register application modules |
| `GetAppDescribe()` | Return application description (displayed when executing -h) |
| `GetAppBuildTime()` | Return build time (displayed when executing -v), typically returns `__DATE__ " " __TIME__` |
| `GetAppVersion(major, minor, rev, build)` | Set application version number |

## Usage Examples

### Single Application

> Full example at `examples/main/01_one_app/`

**Step 1**: Inherit from the Module class

```cpp
// app.h
#include <tbox/main/main.h>

class App : public tbox::main::Module
{
  public:
    App(tbox::main::Context &ctx);
    ~App();

  protected:
    virtual bool onInit(const tbox::Json &cfg) override;
    virtual bool onStart() override;
    virtual void onStop() override;
    virtual void onCleanup() override;
};
```

```cpp
// app.cpp
#include "app.h"
#include <tbox/base/log.h>

App::App(tbox::main::Context &ctx) : Module("app", ctx)
{
    LogTag();
}

bool App::onInit(const tbox::Json &cfg) { LogTag(); return true; }
bool App::onStart() { LogTag(); return true; }
void App::onStop() { LogTag(); }
void App::onCleanup() { LogTag(); }
```

**Step 2**: Implement the registration functions

```cpp
// main.cpp
#include <tbox/main/main.h>
#include "app.h"

namespace tbox {
namespace main {

void RegisterApps(Module &apps, Context &ctx) {
    apps.add(new ::App(ctx));
}

std::string GetAppDescribe() { return "One app sample"; }
std::string GetAppBuildTime() { return __DATE__ " " __TIME__; }

void GetAppVersion(int &major, int &minor, int &rev, int &build) {
    major = 0; minor = 0; rev = 1; build = 0;
}

}}
```

**Step 3**: Add dependency libraries in Makefile

```makefile
LDFLAGS += -L.. \
    -ltbox_main \
    -ltbox_terminal \
    -ltbox_network \
    -ltbox_eventx \
    -ltbox_event \
    -ltbox_util \
    -ltbox_base \
    -lpthread -ldl
```

### Multiple Application Modules

> Full example at `examples/main/02_more_than_one_apps/`

```cpp
void RegisterApps(Module &apps, Context &ctx) {
    apps.add(new App1(ctx));       //! Required startup module
    apps.add(new App2(ctx), false); //! Non-required startup module, failure does not affect other modules
}
```

### Submodule Nesting

Module supports a tree-like nesting structure. The parent module automatically manages the lifecycle of child modules:

```cpp
class ParentApp : public tbox::main::Module {
  public:
    ParentApp(Context &ctx) : Module("parent", ctx) {
        add(new SubModuleA(ctx));      //! Added as a child module
        add(new SubModuleB(ctx));
    }
};

//! The initialize/start/stop/cleanup of child modules are automatically called by the parent module
//! Do not manually delete or call lifecycle methods of child modules
```

### Backend Running Mode

> Full example at `examples/main/06_run_in_backend/`

When you need to integrate the tbox::main framework into an existing program framework, you can use the backend running mode:

```cpp
int main(int argc, char **argv) {
    if (!tbox::main::Start(argc, argv))
        return 0;

    //! The existing program framework continues running
    while (true) {
        // ...
    }

    tbox::main::Stop();
    return 0;
}
```

## Common Scenarios

1. **Standard service programs**: Use `Main()` to run in the foreground, business modules obtain common components via Context
2. **Embedded integration**: Use `Start()/Stop()` to integrate the framework into an existing program without affecting the original architecture
3. **Modular development**: Different functional modules independently inherit from Module, composed via `RegisterApps`
4. **Optional modules**: Use `add(child, false)` to add non-required modules; failure does not affect the main flow

## Important Notes

1. **Module lifecycle order**: Must follow the Construct→initialize→start→stop→cleanup→destruct order; no skipping allowed
2. **Do not manually manage child modules**: After add(), the child module's lifecycle is managed by the parent module; do not manually delete or call lifecycle methods
3. **Logging is unavailable in onFillDefaultConfig**: The logging system is not yet initialized at this stage; do not use LogInfo and similar macros
4. **Impact of the required parameter**: A child module with required=true that fails onInit/onStart will cause the entire program startup to fail
5. **RegisterApps function signature**: Must be placed in the `tbox::main` namespace, otherwise the framework cannot find it

## Related Modules

- **event**: The framework automatically creates a Loop, accessible via `ctx.loop()`
- **eventx**: The framework automatically creates ThreadPool/TimerPool/Async, accessible via `ctx.thread_pool()/ctx.timer_pool()/ctx.async()`
- **terminal**: The framework automatically creates Terminal, accessible via `ctx.terminal()`
- **coroutine**: The framework automatically creates Scheduler, accessible via `ctx.coroutine()`
- **base**: Provides logging, ScopeExit, and other basic components
- **log**: The framework automatically configures the logging system
