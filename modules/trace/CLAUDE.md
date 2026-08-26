# CLAUDE.md - trace 模块

## 模块定位

`trace` 提供函数执行计时的记录与落盘能力，生成二进制 trace 数据，配合 `tools/trace` 工具转换成可在 Chrome `chrome://tracing` 查看的 icicle 图。

## 依赖关系

- 上游依赖：`util`、`base`
- 被依赖：main（可选的 trace 集成）、run

## 关键组件

| 文件 | 说明 |
|------|------|
| `sink.h` | `Sink`（单例）：`setPathPrefix()` 设置输出目录、`enable()`/`disable()`、过滤策略 `FilterStrategy`/`ExemptSet`、`commitRecord()` 提交记录 |

### 与 base 模块的协作

- `base/recorder.h`：`trace::Recorder` 记录单个函数/事件的起止与耗时。
- `base/wrapped_recorder.h`：带 `ENABLE_TRACE_RECORDER` 宏开关的封装，可在编译期彻底关闭 trace。

## 输出目录结构

设置路径前缀如 `/data/my_proc` 后，自动创建 `/data/my_proc.<时间戳>.<pid>/`：
```
|-- names.txt      # 函数名列表
|-- modules.txt    # 模块名列表
|-- threads.txt    # 线程名列表
`-- records/       # 记录文件（如 20240530_041046.bin）
```

## 注意事项

- `Sink` 用 `Sink::GetInstance()` 获取单例。
- 支持按名称过滤（`setFilterStrategy` + `setFilterExemptSet`）与实时落盘开关（`setFileSyncEnable`）。
- 转换工具在 `tools/trace`（非框架库部分）。

## 测试

- 测试文件：`sink_test.cpp`
- 运行：`.build/trace/test`

## 示例

- `examples/trace/01_demo`、`examples/trace/02_multi_threads`
