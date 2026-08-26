# CLAUDE.md - util 模块

## 模块定位

`util` 是一组通用的工具类集合，提供字符串、文件系统、JSON、缓冲区、文件描述符、时间戳等基础工具。

## 依赖关系

- 上游依赖：`base`
- 被依赖：event、eventx、log、network、trace、main、http、jsonrpc 等

## 关键组件

| 文件 | 说明 |
|------|------|
| `argument_parser.h` | 命令行参数解析（`-h`、`--help`、`--level=6`） |
| `split_cmdline.h` | 拆分命令行字串为参数数组 |
| `string.h` | `string::Split()` / `SplitBySpace()` 等字符串操作 |
| `string_to.h` | `StringTo()` 将字串解析为 bool/int/double 等 |
| `fs.h` | 文件系统操作（`FileType`、目录遍历、文件读写等） |
| `json.h` | JSON 字段读取工具（`Get`/`GetField`/`Load`） |
| `json_deep_loader.h` | `DeepLoader` 支持 `__include__` 的 JSON 深度加载器 |
| `variables.h` | `Variables` 变量对象（支持父子层级） |
| `buffer.h` | `Buffer` 缓冲区（读写索引管理） |
| `fd.h` | `Fd` 文件描述符封装（RAII、拷贝/移动语义） |
| `timestamp.h` | 时间戳获取（UTC 秒等） |
| `uuid.h` | UUID 生成 |
| `base64.h` | Base64 编解码 |
| `checksum.h` / `crc.h` | 和校验 / CRC 校验 |
| `serializer.h` | 流式序列化 |
| `scalable_integer.h` | 可变长整数存储与读取 |
| `async_pipe.h` | 异步管道（后台线程处理阻塞写，用于日志落盘等） |
| `execute_cmd.h` | 执行 shell 命令 |
| `pid_file.h` | `PidFile` 进程锁文件 |

## 注意事项

- 该模块所有头文件均以 `tbox/util/` 前缀被引用，如 `#include <tbox/util/buffer.h>`。
- `Variables` 是 flow 模块 Action 配置体系中常用的变量对象。
- `AsyncPipe` 是 trace 模块 `Sink` 与 log 模块异步写文件的底层依赖。

## 测试

- 测试文件：`*_test.cpp`（几乎每个工具都有对应测试）
- 运行：`.build/util/test`
