# D-Bus 集成模块 (dbus)

## 是什么？

dbus 模块提供了 D-Bus（Linux 桌面/系统进程间通信总线）与 cpp-tbox 事件循环的集成能力，让基于 tbox 的服务程序能通过 D-Bus 与其他系统服务交互。

## 为什么需要它？

在 Linux 系统中，D-Bus 是进程间通信的标准机制。许多系统服务（如 systemd、NetworkManager、蓝牙服务等）通过 D-Bus 提供接口。dbus 模块让 tbox 程序能与这些服务交互，同时保持事件驱动的异步模式。

## 头文件

```cpp
#include <tbox/dbus/loop.h>          //! D-Bus 与 event::Loop 的集成
#include <tbox/dbus/connection.h>     //! D-Bus 连接
```

## 核心类与接口

### Connection — D-Bus 连接

| 方法 | 说明 |
|------|------|
| `Connection(loop)` | 构造，指定事件循环 |
| `initialize(bus_type)` | 初始化，指定总线类型（kSession/kSystem/kStarter） |
| `initialize(bus_address)` | 初始化，指定自定义总线地址 |
| `cleanup()` | 清理连接 |

总线类型：
- `kSession` — 会话总线（用户级桌面服务）
- `kSystem` — 系统总线（系统级服务）
- `kStarter` — 启动总线

### AttachLoop / DetachLoop — 挂载/卸载 Loop

```cpp
//! 将已有的 DBusConnection 对象挂载到 event::Loop
dbus::AttachLoop(dbus_conn, sp_loop);

//! 从 event::Loop 上卸载
dbus::DetachLoop(dbus_conn);
```

适用于已有 DBusConnection 的场景（如从 libdbus 直接获取的连接对象）。

## 使用示例

### 基本连接

> 完整示例见 `examples/dbus/00-loop/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/dbus/connection.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

using namespace tbox;
using namespace tbox::event;

int main() {
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();

    dbus::Connection dbus_conn(sp_loop);
    dbus_conn.initialize(dbus::Connection::kSession);  //! 连接会话总线

    //! 此后可使用 dbus_conn 进行 D-Bus 方法调用、信号接收等操作

    sp_loop->runLoop();
    dbus_conn.cleanup();

    LogOutput_Disable();
    return 0;
}
```

### 挂载已有 DBusConnection

```cpp
DBusConnection *raw_conn = dbus_bus_get(DBUS_BUS_SESSION, nullptr);

//! 将原生 D-Bus 连接集成到 tbox 事件循环
dbus::AttachLoop(raw_conn, sp_loop);

//! 此后 D-Bus 事件监听由 tbox Loop 管理

//! 卸载
dbus::DetachLoop(raw_conn);
```

## 常见场景

1. **与系统服务交互**：通过 D-Bus 调用 NetworkManager、systemd 等系统服务接口
2. **桌面应用集成**：与 GNOME/KDE 桌面服务通信
3. **跨进程信号传递**：通过 D-Bus 信号机制在进程间传递事件
4. **硬件状态查询**：查询蓝牙、USB 设备等硬件状态

## 注意事项

1. **依赖 libdbus**：编译时需要链接 dbus-1 库
2. **仅 Linux**：D-Bus 是 Linux 特有的 IPC 机制，不可用于其他平台
3. **Connection 封装**：Connection 对象封装了 DBusConnection 的创建和事件集成，无需手动管理
4. **事件驱动**：D-Bus 事件监听由 Loop 管理，无需额外的 dbus_watch/dbus_timeout 处理
5. **cleanup 顺序**：程序退出前先 cleanup Connection，再 cleanup Loop

## 相关模块

- **event**：基于 Loop 管理 D-Bus 事件监听
- **base**：提供 Log 等基础设施
