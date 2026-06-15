# Performance Trace Module (trace)

## What is it?

The trace module provides lightweight function-level performance tracing functionality, recording the execution time of functions/events and writing trace data to binary files for subsequent visual analysis.

## Why do you need it?

In service applications, understanding which functions execute the slowest and how time is distributed is crucial for performance optimization. The trace module automatically records timestamps and durations at key function entry/exit points without modifying business code, enabling you to obtain detailed performance data.

![trace-view](../images/0011-trace-view.png)

## Header Files

```cpp
#include <tbox/trace/sink.h>
```

## Core Classes and Interfaces

### Sink — Trace Data Receiver

Sink follows the singleton pattern, with a single global instance.

| Method | Description |
|------|------|
| `Sink::GetInstance()` | Get the singleton instance |
| `setPathPrefix(prefix)` | Set path prefix, e.g. "/data/my_proc", automatically creates a subdirectory with timestamp and PID |
| `setFileSyncEnable(enable)` | Set whether to sync data to disk in real time |
| `setRecordFileMaxSize(size)` | Set the maximum record file size |
| `setFilterStrategy(strategy)` | Set filter strategy (kPermit/kReject) |
| `setFilterExemptSet(exempt_set)` | Set the exempt set |
| `enable()` | Enable tracing |
| `disable()` | Disable tracing |
| `isEnabled()` | Check if tracing is enabled |
| `getDirPath()` | Get the directory path |
| `commitRecord(name, module, line, end_ts, duration)` | Commit a trace record |

### Directory Structure

After setting the path prefix, trace automatically creates the following directory structure:

```
/data/my_proc.20240525_123300.7723/
├── names.txt      # Function name list (index-encoded)
├── modules.txt    # Module name list (index-encoded)
├── threads.txt    # Thread name list (index-encoded)
└── records/       # Record file directory
    └── 20240530_041046.bin  # Binary trace record file
```

## Usage Examples

### Basic Tracing

> Full example available in `examples/trace/01_demo/`

```cpp
#include <tbox/trace/sink.h>

//! Enable tracing
auto &sink = tbox::trace::Sink::GetInstance();
sink.setPathPrefix("/data/my_app_trace");
sink.enable();

//! Commit trace records in key functions
void myFunction() {
    uint64_t start_us = /* get start timestamp */;
    //! ... execute business logic ...
    uint64_t end_us = /* get end timestamp */;
    sink.commitRecord("myFunction", "my_module", 0, end_us, end_us - start_us);
}
```

### Multi-thread Tracing

> Full example available in `examples/trace/02_multi_threads/`

```cpp
//! trace supports multi-thread record commits, thread IDs are automatically encoded
//! Backend thread writes to file asynchronously, does not block business threads
```

### Filter Strategy

```cpp
//! Default strategy: record all modules
sink.setFilterStrategy(tbox::trace::Sink::FilterStrategy::kPermit);

//! Reverse strategy: reject specific modules only
sink.setFilterStrategy(tbox::trace::Sink::FilterStrategy::kReject);
sink.setFilterExemptSet({"network", "http"});  //! Exempt these modules
//! Effect: only record trace data for network and http modules
```

## Common Scenarios

1. **Performance Analysis**: Trace execution time of key functions, identify slow functions
2. **Bottleneck Discovery**: Analyze time distribution across modules, find performance bottlenecks
3. **Runtime Monitoring**: Continuously trace program execution, generate a complete execution timeline
4. **Selective Tracing**: Use filter strategies to trace only the modules you care about

## Important Notes

1. **Singleton Pattern**: Sink is obtained via GetInstance(), globally unique, cannot be manually created
2. **Binary File Format**: Record files are in binary format and require a dedicated viewer tool (such as trace_view) for analysis
3. **Path Prefix**: The prefix set by setPathPrefix() automatically gets a timestamp and PID suffix appended
4. **Real-time Disk Sync**: By default, data is not synced to disk in real time (performance priority); enable via setFileSyncEnable(true)
5. **Filter Strategy Combinations**: kPermit + exempt_set = record all by default, only reject modules in exempt_set; kReject + exempt_set = reject all by default, only record modules in exempt_set

## Related Modules

- **util**: Uses AsyncPipe to implement backend thread writing
- **base**: Provides Cabinet, Log and other infrastructure
