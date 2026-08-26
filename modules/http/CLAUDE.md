# CLAUDE.md - http 模块

## 模块定位

`http` 提供 HTTP 服务器与客户端，用于补全服务程序对外提供 RESTful API 的缺口。设计参考 node.js Express 中间件思想。详见 `README.md`。

## 依赖关系

- 上游依赖：`network`、`log`、`eventx`、`event`、`util`、`base`
- 被依赖：websocket

## 关键组件

### 公共

| 文件 | 说明 |
|------|------|
| `common.h` | `Method`、`HttpVer`、`StatusCode` 枚举及转换函数、`Headers` |
| `request.h` | `Request` 请求结构（method/http_ver/url/headers/body） |
| `respond.h` | `Respond` 回复结构（含 `UpgradeCallback` 协议升级回调，用于 WebSocket/SSE） |
| `url.h` | `Url` 结构、`UrlEncode`/`UrlDecode`、`StringToUrl` 等 |

### 服务端 `server/`

| 文件 | 说明 |
|------|------|
| `server/server.h` | `Server`：`use()` 注册中间件、`initialize/start/stop/cleanup`、`setTlsConfig()` |
| `server/context.h` | `Context` 请求上下文（`req()`/`res()`） |
| `server/middlewares/router_middleware.h` | 路由中间件 |
| `server/middlewares/form_data*.h` | 表单数据解析 |
| `server/middlewares/file_downloader_middleware.h` | 文件下载中间件 |
| `server/sse/sse_server.h` | SSE（Server-Sent Events）服务 |

### 客户端 `client/`

- `client/client.h`：`Client` 异步 HTTP 客户端，支持自动重连、请求超时、请求缓存、连接回调。

## 特性（客户端）

| 特性 | 说明 |
|------|------|
| 自动重连 | 断线后自动重连，`setAutoReconnect()` 控制 |
| 请求超时 | 每个请求独立超时定时器，默认 30 秒 |
| 请求缓存 | 未连接时缓存请求，连接后自动发送 |
| 便捷方法 | `request()` 支持 3 种重载 |

## 注意事项

- 服务端中间件返回 `Respond` 后需调用 `next()` 或直接结束；`res()` 在 `done()` 之后不可再用。
- TLS 需链接 `network_tls` 模块，否则 `setTlsConfig()` 无效。

## 测试

- 测试文件：`common_test.cpp`、`respond_test.cpp`、`request_test.cpp`、`url_test.cpp`、`server/request_parser_test.cpp`、`server/sse/*_test.cpp`
- 运行：`.build/http/test`

## 示例

- `examples/http/server`、`examples/http/client`
