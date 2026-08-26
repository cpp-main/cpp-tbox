# CLAUDE.md - mqtt 模块

## 模块定位

`mqtt` 提供 MQTT 客户端，基于 libmosquitto 库，封装为与事件循环集成的 `Client` 类。

## 依赖关系

- 上游依赖：`event`、`base`，以及系统库 `libmosquitto`
- 被依赖：无（独立可选模块）

## 关键组件

- `client.h`：`Client` MQTT 客户端。

### Client 能力

- **状态机**：`kNone → kInited → kConnecting → kTcpConnected → kMqttConnected → kReconnWaiting → kEnd`。
- **配置 `Config`**：
  - `Base`（broker 地址端口、client_id、用户名密码、keepalive）
  - `TLS`（ca_file/ca_path、cert_file/key_file、is_require_peer_cert、ssl_version、is_insecure）
  - `Will`（遗言 topic/payload/qos/retain）
  - 自动重连开关与重连等待时长（可自定义 `auto_reconnect_wait_sec_gen_func`）。
- **回调**：连接成功/失败、断开、消息接收等。

## 注意事项

- 需要系统安装 libmosquitto 开发库（`-lmosquitto`）。
- 使用 `event::Loop` 驱动，所有 I/O 均在 Loop 线程完成。

## 测试

- 无独立测试文件（`TEST_CPP_SRC_FILES` 为空）

## 示例

- `examples/mqtt/`：conn、pub、sub
