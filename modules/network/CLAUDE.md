# CLAUDE.md - network 模块

## 模块定位

`network` 提供通信能力：串口（UART）、UDP、TCP、TLS 配置抽象等。详见 `README`（内容简短：通信模块）。

## 依赖关系

- 上游依赖：`eventx`、`event`、`util`、`base`
- 被依赖：terminal、main、http、network_tls、websocket、run 等

## 关键组件

### 抽象与基础

| 文件 | 说明 |
|------|------|
| `byte_stream.h` | `ByteStream` 字节流接口（`setReceiveCallback`/`send`/`bind`） |
| `buffered_fd.h` | `BufferedFd` 基于 fd 的带缓冲字节流（读写事件驱动） |
| `stdio_stream.h` | 标准输入输出流 |
| `socket_fd.h` | `SocketFd` socket 文件描述符封装（继承 `util::Fd`） |
| `sockaddr.h` | `SockAddr` 地址封装（IPv4 / Unix Local socket，`FromString`） |
| `ip_address.h` | `IPAddress` IP 地址封装 |
| `domain_name.h` | `DomainName` 域名封装 |
| `net_if.h` | `GetNetIF()` 网卡信息查询 |
| `dns_request.h` | `DnsRequest` 异步 DNS 查询（A/CNAME 记录） |

### TCP

| 文件 | 说明 |
|------|------|
| `tcp_connection.h` / `tcp_raw_connection.h` | TCP 连接（`TcpConnection` 带长度头 vs raw 原始字节流） |
| `tcp_acceptor.h` / `tcp_raw_acceptor.h` | TCP 监听接收器 |
| `tcp_connector.h` / `tcp_raw_connector.h` | TCP 连接器（自动重连） |
| `tcp_client.h` | `TcpClient` 客户端（封装 Connector+Connection） |
| `tcp_server.h` | `TcpServer` 服务器（`cabinet::Token` 管理连接） |
| `tcp_factory.h` / `tcp_raw_factory.h` | 连接工厂（创建 acceptor/connector，TLS 工厂的基类） |

### 其它

| 文件 | 说明 |
|------|------|
| `udp_socket.h` | `UdpSocket` UDP 套接字（bind/connect/send/recv） |
| `uart.h` | `Uart` 串口（波特率/数据位/校验位/停止位） |
| `tls_config.h` | `TlsConfig` TLS 配置结构 |
| `tls_factory_entry.h` | `CreateTlsFactory()` 工厂入口（由 network_tls 实现） |

## 注意事项

- `TcpConnection` 与 `TcpRawConnection` 的区别：前者带消息长度头，后者是原始字节流。
- `TcpServer`/`TcpClient` 遵循 `initialize → start → stop → cleanup` 生命期。
- TLS 支持通过 `network_tls` 模块按需链接（`TlsFactoryEntry`）。

## 测试

- 测试文件：`buffered_fd_test.cpp`、`uart_test.cpp`、`ip_address_test.cpp`、`sockaddr_test.cpp`、`udp_socket_test.cpp`、`net_if_test.cpp`、`dns_request_test.cpp`
- 运行：`.build/network/test`

## 示例

- `examples/network/`：buffered_fd、stdio_stream、tcp_acceptor、tcp_client、tcp_connector、tcp_server、uart、udp_socket
