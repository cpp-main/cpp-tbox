# 日志通道模块 (log)

## 是什么？

log 模块提供了日志输出通道（Sink）的多种实现，将日志数据输出到文件、标准输出、系统日志等目标。它基于 base 模块的日志宏（LogInfo/LogErr 等）构建完整的日志系统。

## 为什么需要它？

base/log.h 只定义了日志打印宏和 `LogPrintfFunc` 声明，但没有实现输出功能。log 模块提供了多种 Sink 实现，通过注册到日志系统，将日志数据按指定级别和格式输出到不同目标。

## 头文件

```cpp
#include <tbox/log/sink.h>                //! Sink 基类
#include <tbox/log/async_sink.h>          //! 异步 Sink 基类
#include <tbox/log/async_file_sink.h>     //! 异步文件 Sink
#include <tbox/log/async_stdout_sink.h>   //! 异步 stdout Sink
#include <tbox/log/async_syslog_sink.h>   //! 异步 syslog Sink
#include <tbox/log/sync_stdout_sink.h>    //! 同步 stdout Sink
```

## 核心类与接口

### Sink 类继承层次

```
Sink（基类）
 ├── AsyncSink（异步基类，使用 AsyncPipe）
 │    ├── AsyncFileSink     → 输出到文件
 │    ├── AsyncStdoutSink   → 输出到 stdout
 │    └── AsyncSyslogSink   → 输出到 syslog
 └── SyncStdoutSink         → 同步输出到 stdout
```

### Sink 基类方法

| 方法 | 说明 |
|------|------|
| `setLevel(level)` | 设置默认日志级别过滤 |
| `setLevel(module, level)` | 设置指定模块的日志级别 |
| `unsetLevel(module)` | 取消指定模块的级别设置 |
| `enableColor(enable)` | 启用/禁用彩色输出 |
| `enable()` | 启用 Sink |
| `disable()` | 禁用 Sink |

### AsyncStdoutSink — 异步标准输出

最常用的日志 Sink，将日志异步输出到标准输出。异步模式不会阻塞日志线程。

```cpp
log::AsyncStdoutSink stdout_sink;
stdout_sink.enable();
//! 此后 LogInfo/LogErr 等日志将输出到 stdout
```

### AsyncFileSink — 异步文件输出

将日志异步写入文件，支持按日期自动分割文件。

```cpp
log::AsyncFileSink file_sink;
file_sink.setFilePathPrefix("/data/logs/myapp"); //! 文件路径前缀
file_sink.enable();
```

### SyncStdoutSink — 同步标准输出

简单场景下使用，日志直接输出到 stdout，没有异步管道。适合小型测试程序。

> `LogOutput_Enable()` / `LogOutput_Disable()` 实际上就是创建/销毁一个 SyncStdoutSink。

## 使用示例

### 基本日志输出（最简单方式）

```cpp
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

int main() {
    LogOutput_Enable();    //! 开启日志输出到 stdout

    LogInfo("program started");
    LogErr("some error occurred");

    LogOutput_Disable();   //! 关闭日志输出
    return 0;
}
```

### 配置日志级别过滤

```cpp
//! 只输出 WARN 及以上级别的日志
sink.setLevel(TBOX_LOG_LEVEL_WARN);

//! 为指定模块设置不同的级别
sink.setLevel("network", TBOX_LOG_LEVEL_DEBUG);  //! network 模块输出 DEBUG 级别
sink.setLevel("alarm", TBOX_LOG_LEVEL_INFO);     //! alarm 模块只输出 INFO 及以上
```

### 异步文件日志

```cpp
#include <tbox/log/async_file_sink.h>
#include <tbox/base/log.h>

log::AsyncFileSink file_sink;
file_sink.setFilePathPrefix("/data/logs/myapp");
file_sink.enable();

//! 此后日志将写入文件，文件名自动包含日期和进程号
//! 如：/data/logs/myapp.20240530_123456.12345/
```

### 多 Sink 组合

可同时启用多个 Sink，日志同时输出到多个目标：

```cpp
log::AsyncStdoutSink stdout_sink;
stdout_sink.enable();        //! 输出到 stdout

log::AsyncFileSink file_sink;
file_sink.setFilePathPrefix("/data/logs/app");
file_sink.enable();          //! 输出到文件

//! 日志同时输出到 stdout 和文件
```

## 常见场景

1. **开发调试**：使用 AsyncStdoutSink 输出到终端，级别设为 DEBUG
2. **生产运行**：使用 AsyncFileSink 写入文件，级别设为 INFO
3. **组合输出**：同时启用 stdout + file，终端看实时日志，文件存历史
4. **模块级别控制**：为关键模块设 DEBUG 级别，其他模块设 INFO 级别

## 注意事项

1. **异步 vs 同步**：AsyncSink 使用独立管道线程处理日志，不会阻塞业务线程；SyncStdoutSink 直接在调用线程输出
2. **级别过滤**：默认级别为 MAX（输出所有日志），可根据需要设置
3. **彩色输出**：enableColor(true) 在支持 ANSI 颜色的终端中显示彩色日志
4. **文件分割**：AsyncFileSink 自动按日期分割日志文件
5. **cleanup**：程序退出前调用 cleanup() 确保所有缓冲日志写入完成

## 相关模块

- **base**：提供日志宏（LogInfo/LogErr 等）和 LogPrintfFunc 声明
- **main**：框架自动配置日志系统
- **util**：AsyncPipe 是 AsyncSink 的底层管道组件
