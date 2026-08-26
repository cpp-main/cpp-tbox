# CLAUDE.md - base 模块

## 模块定位

`base` 是整个框架最底层的独立模块，不依赖其它任何 tbox 模块，提供最基础、最通用的基础设施。

## 依赖关系

- 上游依赖：无（仅依赖第三方头文件库 nlohmann/json）
- 被依赖：几乎所有其它模块都依赖 base

## 关键组件

| 文件 | 说明 |
|------|------|
| `log.h` / `log_impl.h` / `log_output.h` | 日志宏与实现，定义 `LogFatal/LogErr/LogWarn/LogNotice/LogImportant/LogInfo/LogDbg/LogTrace` 及日志级别 `TBOX_LOG_LEVEL_*` |
| `json_fwd.h` / `json.hpp` | `tbox::Json`（`nlohmann::json`）与 `tbox::OrderedJson`（`nlohmann::ordered_json`）别名 |
| `scope_exit.hpp` | `SetScopeExitAction(...)` 作用域退出动作宏 |
| `cabinet.hpp` / `cabinet_token.h` | 对象储物柜 `cabinet::Cabinet<T>` 与 `cabinet::Token`（O(1) 存取，Token 失效机制） |
| `object_pool.hpp` | `ObjectPool<T>` 对象池，减少频繁 new/delete |
| `lifetime_tag.hpp` | `LifetimeTag` / `Watcher` 生命期标签，安全跨对象持有指针 |
| `catch_throw.h` | `CatchThrow()` / `CatchThrowQuietly()` 捕获所有异常 |
| `backtrace.h` | `DumpBacktrace()` / `LogBacktrace()` 打印调用栈 |
| `recorder.h` / `wrapped_recorder.h` | 函数耗时记录器（配合 trace 模块），`ENABLE_TRACE_RECORDER` 宏开关 |
| `defines.h` | 常用宏：`NONCOPYABLE`/`IMMOVABLE`、`NUMBER_OF_ARRAY`、资源释放宏等 |
| `assert.h` | `TBOX_ASSERT()` 宏，NDEBUG 下空操作 |
| `memblock.h` | `Memblock` = `std::vector<unsigned char>` |
| `func_types.h` | `VoidFunc` / `BoolFunc` 常用函数类型别名 |
| `version.h` | `GetTboxVersion()` 版本号 |

## 注意事项

- 每个使用日志的模块须定义 `MODULE_ID`（如 `"tbox.base"`），日志宏会自动用作模块标识。
- `json.hpp` 依赖 nlohmann/json 头文件，若缺失需下载并放置到 `/usr/local/include/nlohmann/`。
- `assert.h` 头文件里注释提示：不要在日志相关模块中使用 `TBOX_ASSERT`（避免循环依赖）。
- 模块 ID 通过 Makefile 中 `CXXFLAGS += -DMODULE_ID='"tbox.base"'` 定义。

## 测试

- 测试文件：`*_test.cpp`（log_output、scope_exit、cabinet_token、cabinet、json、lifetime_tag、backtrace、catch_throw、object_pool、recorder 等）
- 运行：`.build/base/test`
