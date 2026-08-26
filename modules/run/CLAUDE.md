# CLAUDE.md - run 模块

## 模块定位

`run` 是框架提供的可执行文件（ELF），通过 `-l` 参数动态加载应用 `lib*.so` 模块并运行，无需为每个应用单独编译 main。

## 依赖关系

- 上游依赖：链接 `main`、`coroutine`、`trace`、`terminal`、`network`、`eventx`、`event`、`log`、`util`、`base`，以及 `-lpthread -ldl -rdynamic`
- 若启用 network_tls，额外以 `--whole-archive` 链接 `-ltbox_network_tls`

## 文件结构

- `main.cpp` — 可执行文件入口，链接后生成 `run` 二进制

## 构建

- `PROJECT = run`，`EXE_NAME = run`，使用 `mk/exe_common.mk`（区别于库模块的 `lib_tbox_common.mk`）

## 注意事项

- `-rdynamic` 用于导出符号，供 dlopen 加载的应用模块回调。
- `--whole-archive -ltbox_network_tls` 确保 `CreateTlsFactory` 注册符号不因未引用被丢弃。

## 示例

- `examples/run/`：echo_server、nc_client、timer_event
