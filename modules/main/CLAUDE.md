# CLAUDE.md - main 模块

## 模块定位

`main` 是应用程序的启动框架，对程序启动/运行/退出过程做了统一完备的封装。详见 `README.md`。

## 依赖关系

- 上游依赖：`terminal`、`network`、`eventx`、`event`、`log`、`util`、`base`、`coroutine`（Context 依赖），以及 `-lpthread -ldl`
- 被依赖：run、以及所有基于 main 框架的应用

## 关键组件

| 文件 | 说明 |
|------|------|
| `module.h` | `Module` 模块类，生命期：`构造 → initialize → start → [运行] → stop → cleanup → 析构`；子类重写 `onInit()`/`onStart()`/`onStop()`/`onCleanup()`，父类提供 `add()` 组合子模块 |
| `context.h` | `Context` 进程上下文：提供 `loop()`、`thread_pool()`、`timer_pool()`、`async()`、`terminal()`、`coroutine()`、`args()` |
| `main.h` | `Main(argc, argv)`（前端阻塞运行）与 `Start(argc, argv)`（后端线程运行） |
| `args_parser.h` | main 框架的命令行参数解析 |

### 实现文件

- `context_imp.cpp` — Context 实现
- `run_in_frontend.cpp` / `run_in_backend.cpp` — 前端/后端运行
- `error_signals.cpp` / `terminate.cpp` — 错误信号处理与终止
- `module.cpp` — 模块生命期逻辑
- `log.cpp` / `trace.cpp` — 日志与 trace 集成
- `misc.cpp` — 杂项

## 应用接入三步走

1. 继承 `tbox::main::Module`，重写 `onInit/onStart/onStop/onCleanup`；
2. 实现 `RegisterApps()`、`GetAppDescribe()`、`GetAppBuildTime()`、`GetAppVersion()`；
3. 链接 `-ltbox_main` 等库，或直接用 `run` 可执行文件 `-l your_lib.so` 加载。

## 注意事项

- 前端 `Main()` 阻塞直到收到 `SIGINT`/`SIGTERM`；`Start()` 在后端线程运行，不阻塞。
- `Context` 是模块获取框架公共资源（事件循环、线程池等）的入口。

## 测试

- 无独立测试文件（`TEST_CPP_SRC_FILES` 为空）

## 示例

- `examples/main/`：00_empty、01_one_app、02_more_than_one_apps、03_nc_client_and_echo_server、04_runtime_error、06_run_in_backend、07_stop_process、sample
