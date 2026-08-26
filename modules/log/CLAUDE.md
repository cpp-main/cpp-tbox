# CLAUDE.md - log 模块

## 模块定位

`log` 提供日志输出通道（Sink）。日志宏定义在 `base/log.h` 中，本模块负责把日志内容输出到不同后端（stdout、syslog、文件）。

## 依赖关系

- 上游依赖：`event`、`util`、`base`
- 被依赖：main、http、websocket、network_tls 等

## 关键组件

| 文件 | 说明 |
|------|------|
| `sink.h` | `Sink` 日志打印通道基类：`setLevel()`（支持按模块过滤）、`enableColor()`、`enable()`/`disable()`、`onLogFrontEnd()` |
| `sync_stdout_sink.h` | 同步 stdout 输出（前端直接打印） |
| `async_sink.h` | 异步 Sink 基类（前端-后端模型，前端提交、后端线程落盘） |
| `async_stdout_sink.h` | 异步 stdout 输出 |
| `async_syslog_sink.h` | 异步 syslog 输出 |
| `async_file_sink.h` | 异步文件输出（支持按大小/时间滚动等） |

## 架构

前端/后端模型：日志宏产生 `LogContent` 后进入 `Sink` 前端，异步 Sink 将数据通过 `util::AsyncPipe` 交给后台线程执行真正的写操作，避免阻塞业务线程。

## 注意事项

- 日志级别宏 `TBOX_LOG_LEVEL_*` 与打印宏 `LogErr` 等定义在 `base/log.h`，不要在本模块重复定义。
- 每个 Sink 都支持按模块（`MODULE_ID`）独立设置日志级别过滤。
- `onLogFrontEnd(const LogContent *content)` 是子类必须实现的纯虚函数。

## 测试

- 测试文件：`async_sink_test.cpp`、`async_stdout_sink_test.cpp`、`async_syslog_sink_test.cpp`、`async_file_sink_test.cpp`、`sync_stdout_sink_test.cpp`
- 运行：`.build/log/test`
