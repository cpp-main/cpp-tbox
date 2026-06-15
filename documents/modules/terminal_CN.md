# 交互终端模块 (terminal)

## 是什么？

terminal 模块提供与运行中程序类似 shell 的交互命令终端。开发或运维人员可通过 telnet 登陆，以命令的方式让程序执行指定函数，实现运行时调试、参数调整、状态查看等功能。

## 为什么需要它？

服务程序运行时通常只能通过日志输出执行过程，无法直接交互。但以下场景非常需要交互能力：
- 开发过程中，想让程序执行某个动作但还没有实现完整的界面
- 程序异常时，希望打印关键信息排查问题
- 突发状态下，运维希望不停止服务就调整运行参数

terminal 的设计模仿 Bash，命令的组织类似文件系统目录树：

```
# tree
|-- dir1
|   |-- dir1_1
|   |   |-- async*
|   |   `-- root(R)
|   `-- dir1_2
|       `-- sync*
|-- dir2
`-- sync*
```

支持 cd, ls, tree, pwd, history, !n, !-n, !! 等常用命令；还支持 UP/DOWN/LEFT/RIGHT/DELETE/HOME/END 按键动作。

## 头文件

```cpp
#include <tbox/terminal/terminal.h>          //! 终端主类
#include <tbox/terminal/terminal_nodes.h>    //! 结点管理接口
#include <tbox/terminal/terminal_interact.h> //! 交互接口
#include <tbox/terminal/helper.h>            //! 辅助函数
#include <tbox/terminal/session.h>           //! 会话管理
#include <tbox/terminal/connection.h>        //! 连接管理
#include <tbox/terminal/types.h>             //! 类型定义
```

## 核心类与接口

### Terminal

Terminal 继承了 TerminalInteract 和 TerminalNodes，同时提供交互能力和结点管理能力。

| 方法 | 说明 |
|------|------|
| `Terminal(loop)` | 构造 |
| `createFuncNode(func, help)` | 创建函数结点 |
| `createDirNode(help)` | 创建目录结点 |
| `deleteNode(token)` | 删除结点 |
| `rootNode()` | 获取根结点 |
| `findNode(path)` | 根据路径查找结点 |
| `mountNode(parent, child, name)` | 将子结点挂载到父目录 |
| `umountNode(parent, name)` | 卸载子结点 |
| `setWelcomeText(text)` | 设置欢迎文字 |

### Func 回调类型

```cpp
using Func = std::function<void(const Session &, const std::vector<std::string> &args)>;
```

函数结点的回调接收 Session 和参数列表。通过 Session 可向客户端输出文字。

### 已验证的 Telnet 客户端

| 客户端 | 说明 |
|--------|------|
| Windows telnet | 自带命令 |
| Linux telnet | 终端命令 |
| Putty | telnet 连接 |
| XShell | telnet 连接 |
| Tabby | telnet profile（Input mode 要设置为 Normal） |

## 使用示例

### 在 main 模块中使用 Terminal

> 完整示例见 `examples/terminal/telnetd/`

```cpp
class App : public tbox::main::Module {
  public:
    App(Context &ctx) : Module("app", ctx) { }

    bool onInit(const Json &cfg) override {
        auto term = ctx.terminal();

        //! 创建函数结点
        auto func_node = term->createFuncNode(
            [](const terminal::Session &s, const std::vector<std::string> &args) {
                s.send("Hello from terminal!\r\n");
            }, "say hello"
        );

        //! 创建目录结点
        auto dir_node = term->createDirNode("demo commands");

        //! 将结点挂载到目录树
        term->mountNode(term->rootNode(), dir_node, "demo");
        term->mountNode(dir_node, func_node, "hello");

        //! 设置欢迎文字
        term->setWelcomeText("Welcome to my app terminal!\r\n");

        return true;
    }
};
```

### 通过 Terminal 启动 Telnet 服务

> 完整示例见 `examples/terminal/telnetd/`

terminal 通常配合 TcpAcceptor 提供 telnet 服务：

```cpp
//! 在 main 模块中，Terminal 已由框架自动创建
//! 只需创建 TcpAcceptor 监听端口，并处理连接
```

### Stdio 终端

> 完整示例见 `examples/terminal/stdio/`

也可通过标准输入输出与 terminal 交互，无需 telnet：

```cpp
//! 使用 StdioStream 将 stdin/stdout 连接到 Terminal
```

## 常见场景

1. **运行时调试**：创建函数结点打印关键变量值
2. **参数调整**：创建函数结点修改运行参数（如日志级别、定时器间隔）
3. **状态查看**：创建函数结点返回系统状态信息
4. **远程运维**：通过 telnet 远程连接，无需停机调整

## 注意事项

1. **Terminal 由 main 框架创建**：在 main 模块中使用时，通过 `ctx.terminal()` 获取，无需手动创建
2. **结点路径命名**：建议使用有意义的英文名称，便于命令行输入
3. **回调线程安全**：Terminal 回调在 Loop 线程中执行，与业务逻辑同线程
4. **Func 回调的 Session**：通过 Session.send() 向客户端输出文字，需包含 `\r\n` 换行

## 相关模块

- **event**：Terminal 基于 Loop 运行
- **network**：通过 TcpAcceptor 提供 telnet 服务连接
- **main**：框架自动创建 Terminal 对象
