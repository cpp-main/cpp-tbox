# CLAUDE.md - network_tls 模块

## 模块定位

`network_tls` 为 network 模块提供 TLS/SSL 加密通信能力，基于 OpenSSL。通过 `TcpTlsFactory` 实现 TLS 版的 acceptor/connector/connection。

## 依赖关系

- 上游依赖：`network`、`log`、`eventx`、`event`、`util`、`base`，以及系统库 `libssl`、`libcrypto`
- 被依赖：可选（config.mk 中按需启用），http/websocket 通过 `TlsConfig` 使用

## 关键组件

| 文件 | 说明 |
|------|------|
| `tls_factory_entry.cpp` | 实现 `network::CreateTlsFactory()` 工厂入口（供 network 模块调用） |
| `tcp_tls_factory.h` | `TcpTlsFactory` TLS 工厂：根据 `TlsRole` 创建 `SSL_CTX`，kClient 只支持 createConnector，kServer 只支持 createAcceptor |
| `tcp_tls_connection.h` | `TcpTlsConnection` TLS 加密连接（基于 `BufferedSslFd`） |
| `tcp_tls_connector.h` | `TcpTlsConnector` TLS 连接器（TCP 连接成功后先 SSL 握手） |
| `tcp_tls_acceptor.h` | `TcpTlsAcceptor` TLS 接收器（accept 后 SSL 握手） |
| `buffered_ssl_fd.h` | `BufferedSslFd` 基于 SSL 的 BufferedFd（`doRead`/`doWrite` 使用 SSL_read/SSL_write） |

## 架构

SSL 握手由 `TcpTlsConnector`/`TcpTlsAcceptor` 在创建连接对象之前完成；`BufferedSslFd` 只负责已建立 SSL 连接的 I/O，并处理 `WANT_READ/WANT_WRITE`（renegotiation）。

## 注意事项

- 该模块是**可选模块**，config.mk 中注释/取消注释 `MODULES += network_tls` 即可启用/关闭，关闭后不链接 `libssl/libcrypto`。
- `run` 模块链接时通过 `--whole-archive` 强制链接本模块，以保证 `CreateTlsFactory` 注册生效。
- `TcpTlsConnection` 接管 SSL 对象生命期，析构时 `SSL_free()`。

## 测试

- 暂无独立测试（`TEST_CPP_SRC_FILES` 为空）

## 示例

- 无独立示例，可参考 http/websocket 的 TLS 用法。
