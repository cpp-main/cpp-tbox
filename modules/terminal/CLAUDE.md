# CLAUDE.md - terminal 模块

## 模块定位

`terminal` 提供一个类似 shell 的交互命令终端，让开发/运维人员可在程序运行时通过 telnet 等方式登陆执行命令。详见 `README.md`。

## 依赖关系

- 上游依赖：`network`、`event`、`util`、`base`
- 被依赖：main（Context 暴露 `TerminalNodes`）、run

## 关键组件

| 文件 | 说明 |
|------|------|
| `terminal.h` | `Terminal` 终端核心类（继承 `TerminalInteract` + `TerminalNodes`，会话与结点管理） |
| `terminal_interact.h` | 终端交互接口（newSession/onRecvString/onExit 等） |
| `terminal_nodes.h` | 结点管理接口（createFuncNode/createDirNode/mountNode/umountNode） |
| `session.h` | `Session` 会话对象（发送数据/结束会话） |
| `types.h` | 类型定义（`SessionToken`/`NodeToken` = `cabinet::Token`，`Func`） |
| `helper.h` | 便捷添加结点函数（`AddDirNode`/`AddFuncNode`，支持 bool/int/double/string 变量读写） |
| `connection.h` | 连接抽象 |
| `service/telnetd.h` | `Telnetd` telnet 服务 |
| `service/tcp_rpc.h` | `TcpRpc` TCP 命令服务 |
| `service/stdio.h` | `Stdio` 标准输入输出终端 |

### 实现目录 `impl/`

- `terminal.cpp` / `terminal_commands.cpp` / `terminal_key_events.cpp` — 命令与按键处理
- `dir_node.cpp` / `func_node.cpp` / `node.h` — 目录/函数结点
- `key_event_scanner.cpp` — 按键事件扫描（含测试）
- `service/` — 各服务实现

## 特性

命令组织类似文件系统目录树，支持 `cd`、`ls`、`tree`、`pwd`、`history`、`!n`、`!-n`、`!!`，以及 UP/DOWN/LEFT/RIGHT/DELETE/HOME/END 按键。

## 注意事项

- 命令结点分两类：`FuncNode`（函数）与 `DirNode`（目录），通过 mount/umount 组织层级。
- 已通过测试的 telnet 客户端：Windows telnet、Linux telnet、Putty、XShell、Tabby（Input mode 需 Normal）。

## 测试

- 测试文件：`impl/key_event_scanner_test.cpp`
- 运行：`.build/terminal/test`

## 示例

- `examples/terminal/`：telnetd、tcp_rpc、stdio、build_nodes.cpp
