# Interactive Terminal Module (terminal)

## What is it?

The terminal module provides a shell-like interactive command terminal for running programs. Developers or operations personnel can log in via telnet and use commands to instruct the program to execute specified functions, enabling runtime debugging, parameter adjustment, status inspection, and more.

## Why do you need it?

Service programs at runtime typically only expose their execution process through log output, with no direct interaction. However, the following scenarios strongly require interactive capability:
- During development, you want the program to perform an action but haven't yet implemented a complete UI
- When the program behaves abnormally, you want to print key information to troubleshoot the issue
- Under unexpected conditions, ops personnel want to adjust runtime parameters without stopping the service

The terminal design mimics Bash, with commands organized like a filesystem directory tree:

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

It supports common commands such as cd, ls, tree, pwd, history, !n, !-n, !!; it also supports UP/DOWN/LEFT/RIGHT/DELETE/HOME/END key actions.

## Header Files

```cpp
#include <tbox/terminal/terminal.h>          //! Terminal main class
#include <tbox/terminal/terminal_nodes.h>    //! Node management interface
#include <tbox/terminal/terminal_interact.h> //! Interaction interface
#include <tbox/terminal/helper.h>            //! Helper functions
#include <tbox/terminal/session.h>           //! Session management
#include <tbox/terminal/connection.h>        //! Connection management
#include <tbox/terminal/types.h>             //! Type definitions
```

## Core Classes and Interfaces

### Terminal

Terminal inherits from TerminalInteract and TerminalNodes, providing both interaction capability and node management capability.

| Method | Description |
|--------|-------------|
| `Terminal(loop)` | Constructor |
| `createFuncNode(func, help)` | Create a function node |
| `createDirNode(help)` | Create a directory node |
| `deleteNode(token)` | Delete a node |
| `rootNode()` | Get the root node |
| `findNode(path)` | Find a node by path |
| `mountNode(parent, child, name)` | Mount a child node onto a parent directory |
| `umountNode(parent, name)` | Unmount a child node |
| `setWelcomeText(text)` | Set welcome text |

### Func Callback Type

```cpp
using Func = std::function<void(const Session &, const std::vector<std::string> &args)>;
```

The function node callback receives a Session and an argument list. Through Session you can output text to the client.

### Verified Telnet Clients

| Client | Description |
|--------|-------------|
| Windows telnet | Built-in command |
| Linux telnet | Terminal command |
| Putty | telnet connection |
| XShell | telnet connection |
| Tabby | telnet profile (Input mode must be set to Normal) |

## Usage Examples

### Using Terminal in a main Module

> See `examples/terminal/telnetd/` for a complete example

```cpp
class App : public tbox::main::Module {
  public:
    App(Context &ctx) : Module("app", ctx) { }

    bool onInit(const Json &cfg) override {
        auto term = ctx.terminal();

        //! Create a function node
        auto func_node = term->createFuncNode(
            [](const terminal::Session &s, const std::vector<std::string> &args) {
                s.send("Hello from terminal!\r\n");
            }, "say hello"
        );

        //! Create a directory node
        auto dir_node = term->createDirNode("demo commands");

        //! Mount nodes into the directory tree
        term->mountNode(term->rootNode(), dir_node, "demo");
        term->mountNode(dir_node, func_node, "hello");

        //! Set welcome text
        term->setWelcomeText("Welcome to my app terminal!\r\n");

        return true;
    }
};
```

### Starting a Telnet Service via Terminal

> See `examples/terminal/telnetd/` for a complete example

terminal is typically used together with TcpAcceptor to provide a telnet service:

```cpp
//! In the main module, Terminal is automatically created by the framework
//! Just create a TcpAcceptor to listen on a port and handle connections
```

### Stdio Terminal

> See `examples/terminal/stdio/` for a complete example

You can also interact with terminal via standard input/output, without needing telnet:

```cpp
//! Use StdioStream to connect stdin/stdout to Terminal
```

## Common Scenarios

1. **Runtime debugging**: Create function nodes to print key variable values
2. **Parameter adjustment**: Create function nodes to modify runtime parameters (e.g., log level, timer interval)
3. **Status inspection**: Create function nodes that return system status information
4. **Remote ops**: Connect remotely via telnet and adjust without downtime

## Important Notes

1. **Terminal is created by the main framework**: When used in a main module, obtain it via `ctx.terminal()`; no need to create it manually
2. **Node path naming**: Use meaningful English names for ease of command-line input
3. **Callback thread safety**: Terminal callbacks execute in the Loop thread, on the same thread as business logic
4. **Session in Func callbacks**: Use Session.send() to output text to the client; include `\r\n` for line breaks

## Related Modules

- **event**: Terminal runs on Loop
- **network**: Provides telnet service connections via TcpAcceptor
- **main**: The framework automatically creates the Terminal object
