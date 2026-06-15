# Log Sink Module (log)

## What is it?

The log module provides multiple implementations of log output sinks, routing log data to destinations such as files, standard output, and the system log. It builds a complete logging system on top of the log macros (LogInfo/LogErr, etc.) from the base module.

## Why do you need it?

base/log.h only defines the log printing macros and the `LogPrintfFunc` declaration, but does not implement output functionality. The log module provides multiple Sink implementations that, when registered into the logging system, output log data to different destinations at specified levels and formats.

## Header Files

```cpp
#include <tbox/log/sink.h>                //! Sink base class
#include <tbox/log/async_sink.h>          //! Async Sink base class
#include <tbox/log/async_file_sink.h>     //! Async file Sink
#include <tbox/log/async_stdout_sink.h>   //! Async stdout Sink
#include <tbox/log/async_syslog_sink.h>   //! Async syslog Sink
#include <tbox/log/sync_stdout_sink.h>    //! Sync stdout Sink
```

## Core Classes and Interfaces

### Sink Class Hierarchy

```
Sink (base class)
 ├── AsyncSink (async base class, uses AsyncPipe)
 │    ├── AsyncFileSink     → output to file
 │    ├── AsyncStdoutSink   → output to stdout
 │    └── AsyncSyslogSink   → output to syslog
 └── SyncStdoutSink         → sync output to stdout
```

### Sink Base Class Methods

| Method | Description |
|--------|-------------|
| `setLevel(level)` | Set the default log level filter |
| `setLevel(module, level)` | Set the log level for a specific module |
| `unsetLevel(module)` | Remove the level setting for a specific module |
| `enableColor(enable)` | Enable/disable colored output |
| `enable()` | Enable the Sink |
| `disable()` | Disable the Sink |

### AsyncStdoutSink — Async Standard Output

The most commonly used log Sink, which outputs logs asynchronously to standard output. The async mode does not block the logging thread.

```cpp
log::AsyncStdoutSink stdout_sink;
stdout_sink.enable();
//! After this, LogInfo/LogErr and other logs will output to stdout
```

### AsyncFileSink — Async File Output

Writes logs asynchronously to a file, with automatic date-based file splitting.

```cpp
log::AsyncFileSink file_sink;
file_sink.setFilePathPrefix("/data/logs/myapp"); //! File path prefix
file_sink.enable();
```

### SyncStdoutSink — Sync Standard Output

Used in simple scenarios where logs are output directly to stdout without an async pipeline. Suitable for small test programs.

> `LogOutput_Enable()` / `LogOutput_Disable()` essentially creates/destroys a SyncStdoutSink.

## Usage Examples

### Basic Log Output (Simplest Approach)

```cpp
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

int main() {
    LogOutput_Enable();    //! Enable log output to stdout

    LogInfo("program started");
    LogErr("some error occurred");

    LogOutput_Disable();   //! Disable log output
    return 0;
}
```

### Configuring Log Level Filtering

```cpp
//! Only output logs at WARN level and above
sink.setLevel(TBOX_LOG_LEVEL_WARN);

//! Set different levels for specific modules
sink.setLevel("network", TBOX_LOG_LEVEL_DEBUG);  //! network module outputs DEBUG level
sink.setLevel("alarm", TBOX_LOG_LEVEL_INFO);     //! alarm module only outputs INFO and above
```

### Async File Logging

```cpp
#include <tbox/log/async_file_sink.h>
#include <tbox/base/log.h>

log::AsyncFileSink file_sink;
file_sink.setFilePathPrefix("/data/logs/myapp");
file_sink.enable();

//! After this, logs will be written to files whose names automatically include the date and process ID
//! e.g.: /data/logs/myapp.20240530_123456.12345/
```

### Multi-Sink Composition

You can enable multiple Sinks simultaneously, outputting logs to multiple destinations at the same time:

```cpp
log::AsyncStdoutSink stdout_sink;
stdout_sink.enable();        //! Output to stdout

log::AsyncFileSink file_sink;
file_sink.setFilePathPrefix("/data/logs/app");
file_sink.enable();          //! Output to file

//! Logs now output to both stdout and file
```

## Common Scenarios

1. **Development debugging**: Use AsyncStdoutSink to output to the terminal, set level to DEBUG
2. **Production deployment**: Use AsyncFileSink to write to files, set level to INFO
3. **Combined output**: Enable both stdout + file simultaneously; view real-time logs on the terminal and store history in files
4. **Module-level control**: Set DEBUG level for key modules and INFO level for other modules

## Important Notes

1. **Async vs Sync**: AsyncSink uses a dedicated pipe thread to process logs and does not block business threads; SyncStdoutSink outputs directly in the calling thread
2. **Level filtering**: The default level is MAX (outputs all logs), which can be adjusted as needed
3. **Colored output**: enableColor(true) displays colored logs in terminals that support ANSI colors
4. **File splitting**: AsyncFileSink automatically splits log files by date
5. **Cleanup**: Call cleanup() before program exit to ensure all buffered logs are fully written

## Related Modules

- **base**: Provides log macros (LogInfo/LogErr, etc.) and the LogPrintfFunc declaration
- **main**: The framework automatically configures the logging system
- **util**: AsyncPipe is the underlying pipeline component used by AsyncSink
