# CLAUDE.md - jsonrpc 模块

## 模块定位

`jsonrpc` 提供 JSON-RPC 协议的编解码与 RPC 调用框架，支持多种传输层分包协议。

## 依赖关系

- 上游依赖：`event`、`util`、`base`（`Rpc` 使用 `eventx::TimeoutMonitor` 头文件做请求超时）
- 被依赖：无（独立可选模块）

## 关键组件

| 文件 | 说明 |
|------|------|
| `types.h` | `ErrorCode`（-32700 解析错误、-32600 非法请求等）、`IdType`（整数/字符串 id）、`Response` |
| `proto.h` | `Proto` 协议基类：`sendRequest/sendResult/sendError`（支持 int/string id）、`setRecvCallback`、纯虚 `onRecvData()`/`sendJson()` |
| `rpc.h` | `Rpc` RPC 框架：`addService()` 注册方法、`request()`/`notify()` 发送请求/通知、请求超时 |
| `protos/raw_stream_proto.h` | `RawStreamProto` 裸流协议（按 `{}`/`[]` 计数界定 JSON，适用于 TCP） |
| `protos/header_stream_proto.h` | `HeaderStreamProto` 含头部流协议（`[2B Head + 4B Length + JSON]`，适用于 TCP） |
| `protos/packet_proto.h` | `PacketProto` 分包协议（每次 `onRecvData()` 即一个完整 JSON，适用于 UDP/MQTT/HTTP） |

## 架构

`Proto` 负责 JSON 编解码与分包，`Rpc` 负责方法注册、请求-回复匹配与超时管理。两者通过回调桥接。

## 注意事项

- 协议 `onRecvData()` 返回消费的字节数，用于处理粘包/半包。
- `Rpc::addService` 回调返回 `true` 为同步回复，`false` 为异步回复（稍后调用 `respond()`）。
- id 类型可用整数或字符串（`StrIdGenFunc` 生成 uuid 类字符串 id）。

## 测试

- 测试文件：`protos/raw_stream_proto_test.cpp`、`protos/header_stream_proto_test.cpp`、`protos/packet_proto_test.cpp`、`rpc_int_test.cpp`、`rpc_str_test.cpp`
- 运行：`.build/jsonrpc/test`

## 示例

- `examples/jsonrpc/`：message、req_rsp
