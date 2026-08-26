# CLAUDE.md - websocket 模块

## 模块定位

`websocket` 提供 WebSocket 服务器与客户端。服务器基于 HTTP 服务器运行（本身即为 HTTP 中间件），支持 RFC 6455 帧协议、分片、压缩（permessage-deflate）、Ping/Pong 保活。

## 依赖关系

- 上游依赖：`crypto`（SHA1 握手）、`http`、`network`、`log`、`eventx`、`event`、`util`、`base`，以及 `-lz`（压缩）
- 被依赖：无（独立可选模块）

## 关键组件

### 帧处理

| 文件 | 说明 |
|------|------|
| `ws_frame.h` | WebSocket 帧结构定义 |
| `ws_frame_parser.h` | 帧解析器（含分片消息组装） |
| `ws_frame_builder.h` | 帧构造器（含分片发送） |
| `ws_compressor.h` | permessage-deflate 压缩/解压 |

### 服务端 `server/`

- `server/ws_server.h`：`WsServer` 服务器（`initialize(http_server, url_path)` 关联 HTTP 服务器、URL 路径匹配、`ConnToken` 管理连接、`send`/`close`/`ping`）

### 客户端 `client/`

- `client/ws_client.h`：`WsClient` 客户端（`initialize(server_addr, url_path)`、自动重连、掩码帧、TLS 支持）

## 特性

- 分片消息接收完整后统一解压再回调，使用右值引用（`std::string&&` / `std::vector<uint8_t>&&`）提升效率。
- 服务器 URL 路径匹配规则：以 `/` 结尾为前缀匹配，否则全量匹配，空串匹配所有。
- 默认分片大小 `kDefaultFragmentSize = 65535`，`0` 表示不分片。
- `setPingInterval`/`setPingTimeout` 实现保活与超时检测。

## 注意事项

- 客户端帧必须掩码，服务器帧不掩码。
- 压缩与分片大小须在 `initialize()` 之前调用。
- TLS 需链接 `network_tls` 模块，否则 `setTlsConfig()` 无效。

## 测试

- 测试文件：`ws_frame_parser_test.cpp`、`ws_frame_builder_test.cpp`、`ws_compressor_test.cpp`、`server/ws_server_impl_test.cpp`
- 运行：`.build/websocket/test`

## 示例

- `examples/websocket/`：chat_client、chat_server、echo_bin
