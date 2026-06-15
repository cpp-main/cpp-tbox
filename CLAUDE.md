# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

cpp-tbox (C++ Treasure Box) is a Reactor-based service development framework and component library for Linux, targeting intelligent hardware, edge computing, and backend services. Version 1.13.x, C++11, MIT license.

## Build Commands

### GNU Make (primary)
```bash
# Build everything (3rd-party + all modules in config.mk)
make 3rd-party modules RELEASE=1

# Build with ASAN (recommended, enables memory leak checking)
make 3rd-party modules ENABLE_ASAN=1

# Build and run tests for all enabled modules
make test ENABLE_ASAN=1
make run_test ENABLE_ASAN=1   # executes each module's test binary

# Build single module only
make -C modules/base ENABLE_ASAN=1

# Build and test single module
make -C modules/base test ENABLE_ASAN=1

# Clean
make clean          # rm .build
make distclean      # rm .build .staging .install

# Custom staging directory
make 3rd-party modules RELEASE=1 STAGING_DIR=$HOME/.tbox
```

> **Note:** Always add `ENABLE_ASAN=1` when building with make to enable AddressSanitizer for memory leak detection.

### CMake (alternative)
```bash
cmake -B build
cmake --build build
cmake --install build            # default: /usr/local
cmake --install build --prefix=$HOME/.tbox  # custom prefix

# Run tests
cd build && ctest
```

### Module selection
Edit `config.mk` to enable/disable modules. Add `MODULES += xxx` or comment it out. Core modules (base, util, event, eventx, log, network, terminal, trace, coroutine, main, run) are always enabled.

## Testing

Each module has a `xxx_test.cpp` alongside `xxx.cpp`. Tests use gmock/gtest framework. Test binaries are built into `.build/<module>/test`.

Run a single module's test: `make -C modules/<name> test ENABLE_ASAN=1 && .build/<name>/test`

## Architecture

### Module dependency hierarchy (bottom-up)
- **base** → standalone (logging macros, backtrace, json, scope\_exit, cabinet, object\_pool)
- **util** → depends on base (argument\_parser, fs, variables, etc.)
- **event** → depends on base (Loop, FdEvent, TimerEvent, SignalEvent; engines: epoll, select)
- **eventx** → depends on event (ThreadPool, TimerPool, Async, WorkThread)
- **log** → depends on event (stdout/syslog/filelog output channels with async frontend-backend model)
- **network** → depends on event+log (serial, terminal, UDP, TCP)
- **terminal** → depends on event+network (shell-like command terminal for runtime interaction)
- **trace** → depends on event (function execution timing recorder, exports icicle diagrams)
- **coroutine** → depends on event (Scheduler, coroutine for sequential async flows)
- **main** → depends on event+eventx+terminal+coroutine (Module lifecycle framework, Context provides loop/thread\_pool/timer\_pool/terminal/coroutine)
- **run** → depends on main (ELF executable that loads lib\*.so modules via `-l` parameter)
- **http/mqtt/flow/alarm/crypto/dbus/jsonrpc** → optional, depend on various core modules

### Module lifecycle
Every `tbox::main::Module` follows: **construct → onInit() → onStart() → [running] → onStop() → onCleanup() → destruct**. Modules support parent-child composition via `add()`. Required children must succeed; optional children can fail without blocking the app.

### Reactor pattern
`event::Loop` is the core. Main thread runs `loop->runLoop(Mode::kForever)` handling non-blocking IO/timer/signal events. Cross-thread delegation via `runInLoop()` (thread-safe) or `runNext()` (loop-thread only, faster). `run()` auto-selects based on thread context.

### Logging
`LogFatal/LogErr/LogWarn/LogNotice/LogImportant/LogInfo/LogDbg/LogTrace` macros defined in `base/log.h`. Each module defines `MODULE_ID` (e.g. `"tbox.base"`) which becomes the log module identifier. Log levels prefixed with `TBOX_LOG_LEVEL_` to avoid conflicts with other libraries.

### How to create a new app
1. Derive from `tbox::main::Module`, implement `onInit/onStart/onStop/onCleanup`
2. In `main.cpp`, implement `RegisterApps()`, `GetAppDescribe()`, `GetAppBuildTime()`, `GetAppVersion()`
3. Call `tbox::main::Main(argc, argv)` — or use `run` executable with `-l your_lib.so`

## Code Style

Follow Google C++ style with these exceptions:
- Source file extension: `.cpp` (not `.cc`)
- Indentation: 4 spaces
- Member function naming: `aaaBbb()` (lowerCamelCase)
- Static function naming: `AaaBbb()` (UpperCamelCase)
- Static variable naming: `_xxx_` (underscore-prefixed-and-suffixed)
- Static variable (local): `_xxx` (underscore-prefixed)
- Avoid smart pointers unless necessary
- File format: Unix, encoding: UTF-8
- Chinese comments are common and expected

## File header

Every source file starts with the ASCII art logo block + copyright notice. Preserve it when modifying files.

## PR conventions

- PRs go to `develop` branch, not `master`
- New components must include: `.cpp`, `.h`, `_test.cpp`, and a sample/example
