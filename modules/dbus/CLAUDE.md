# CLAUDE.md - dbus 模块

## 模块定位

`dbus` 提供 D-Bus 连接的封装，以及将 D-Bus 连接挂载到 `event::Loop` 的能力。

## 依赖关系

- 上游依赖：`event`、`base`，以及系统库 `libdbus-1`（通过 `pkg-config --cflags/--libs dbus-1`）
- 被依赖：无（独立可选模块）

## 关键组件

| 文件 | 说明 |
|------|------|
| `connection.h` | `Connection` D-Bus 连接封装：`initialize(BusType)`（Session/System/Starter）或 `initialize(bus_address)`、`cleanup()` |
| `loop.h` | `AttachLoop(DBusConnection*, event::Loop*)` / `DetachLoop(DBusConnection*)` 将原生 D-Bus 连接挂载到事件循环 |

## 注意事项

- 需要系统安装 libdbus-1-dev。
- `AttachLoop`/`DetachLoop` 直接操作原生 `DBusConnection*`，可与 `Connection` 类配合使用。

## 测试

- 无测试文件

## 示例

- `examples/dbus/00-loop`
