# 性能追踪模块 (trace)

## 是什么？

trace 模块提供了轻量级函数级性能追踪功能，记录函数/事件的执行时间，将追踪数据写入二进制文件供后续可视化分析。

## 为什么需要它？

在服务程序运行中，了解哪些函数执行最慢、耗时分布如何，对于性能优化至关重要。trace 模块通过在关键函数入口/出口自动记录时间戳和耗时，无需修改业务代码，就能获取详细的性能数据。

![trace-view](../images/0011-trace-view.png)

## 头文件

```cpp
#include <tbox/trace/sink.h>
```

## 核心类与接口

### Sink — 追踪数据接收器

Sink 是单例模式，全局唯一实例。

| 方法 | 说明 |
|------|------|
| `Sink::GetInstance()` | 获取单例实例 |
| `setPathPrefix(prefix)` | 设置路径前缀，如 "/data/my_proc"，自动创建带时间戳和PID的子目录 |
| `setFileSyncEnable(enable)` | 设置是否实时落盘 |
| `setRecordFileMaxSize(size)` | 设置记录文件大小上限 |
| `setFilterStrategy(strategy)` | 设置过滤策略（kPermit/kReject） |
| `setFilterExemptSet(exempt_set)` | 设置豁免集合 |
| `enable()` | 启用追踪 |
| `disable()` | 停用追踪 |
| `isEnabled()` | 是否已启用 |
| `getDirPath()` | 获取目录路径 |
| `commitRecord(name, module, line, end_ts, duration)` | 提交一条追踪记录 |

### 目录结构

设置路径前缀后，trace 自动创建如下目录结构：

```
/data/my_proc.20240525_123300.7723/
├── names.txt      # 函数名列表（索引编码）
├── modules.txt    # 模块名列表（索引编码）
├── threads.txt    # 线程名列表（索引编码）
└── records/       # 记录文件目录
    └── 20240530_041046.bin  # 二进制追踪记录文件
```

## 使用示例

### 基本追踪

> 完整示例见 `examples/trace/01_demo/`

```cpp
#include <tbox/trace/sink.h>

//! 启用追踪
auto &sink = tbox::trace::Sink::GetInstance();
sink.setPathPrefix("/data/my_app_trace");
sink.enable();

//! 在关键函数中提交追踪记录
void myFunction() {
    uint64_t start_us = /* 获取开始时间戳 */;
    //! ... 执行业务逻辑 ...
    uint64_t end_us = /* 获取结束时间戳 */;
    sink.commitRecord("myFunction", "my_module", 0, end_us, end_us - start_us);
}
```

### 多线程追踪

> 完整示例见 `examples/trace/02_multi_threads/`

```cpp
//! trace 支持多线程提交记录，线程号自动编码
//! 后端线程异步写入文件，不阻塞业务线程
```

### 过滤策略

```cpp
//! 默认策略：记录所有模块
sink.setFilterStrategy(tbox::trace::Sink::FilterStrategy::kPermit);

//! 反向策略：只拒绝特定模块
sink.setFilterStrategy(tbox::trace::Sink::FilterStrategy::kReject);
sink.setFilterExemptSet({"network", "http"});  //! 豁免这些模块
//! 效果：只记录 network 和 http 模块的追踪数据
```

## 常见场景

1. **性能分析**：追踪关键函数的执行耗时，定位慢函数
2. **瓶颈发现**：统计各模块的耗时分布，找到性能瓶颈
3. **运行时监控**：持续追踪程序运行过程，生成完整的执行时间线
4. **选择性追踪**：使用过滤策略仅追踪关心的模块

## 注意事项

1. **单例模式**：Sink 使用 GetInstance() 获取，全局唯一，不可手动创建
2. **二进制文件格式**：记录文件为二进制格式，需要专门的查看工具（如 trace_view）分析
3. **路径前缀**：setPathPrefix() 设置的前缀会自动添加时间戳和PID后缀
4. **实时落盘**：默认不实时落盘（性能优先），可通过 setFileSyncEnable(true) 启用
5. **过滤策略组合**：kPermit + exempt_set = 默认记录所有，仅拒绝 exempt_set 中的模块；kReject + exempt_set = 默认拒绝所有，仅记录 exempt_set 中的模块

## 相关模块

- **util**：使用 AsyncPipe 实现后端线程写入
- **base**：提供 Cabinet、Log 等基础设施
