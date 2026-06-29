# 网络通信模块 (network)

## 是什么？

network 模块基于 event 模块提供了 TCP/UDP/UART 通信能力，包括服务端（TcpServer/TcpAcceptor）、客户端（TcpClient/TcpConnector）、UDP 通信（UdpSocket）、串口通信（Uart）以及字节流抽象（ByteStream）。

## 为什么需要它？

在服务型程序中，网络通信是最基础的需求。但传统 socket 编程需要处理大量细节：fd 管理、事件监听、数据缓冲、连接管理等。network 模块将这些封装为面向对象的接口，与事件循环无缝集成，开发者只需关注业务逻辑。

## 头文件

```cpp
#include <tbox/network/tcp_server.h>      //! TCP 服务端
#include <tbox/network/tcp_acceptor.h>    //! TCP 连接接收器
#include <tbox/network/tcp_connector.h>   //! TCP 连接器（带自动重连）
#include <tbox/network/tcp_client.h>      //! TCP 客户端（封装连接+通信）
#include <tbox/network/tcp_connection.h>  //! TCP 连接（底层）
#include <tbox/network/udp_socket.h>      //! UDP 套接字
#include <tbox/network/uart.h>            //! 串口通信
#include <tbox/network/byte_stream.h>     //! 字节流抽象接口
#include <tbox/network/buffered_fd.h>     //! 带缓冲的 fd
#include <tbox/network/sockaddr.h>        //! 地址封装
#include <tbox/network/socket_fd.h>       //! 套接字 fd
#include <tbox/network/ip_address.h>      //! IP 地址
#include <tbox/network/dns_request.h>     //! DNS 请求
#include <tbox/network/domain_name.h>     //! 域名解析
#include <tbox/network/net_if.h>          //! 网络接口
#include <tbox/network/stdio_stream.h>    //! 标准 I/O 流
#include <tbox/network/tls_config.h>      //! TLS 配置
```

## 核心类与接口

### TCP 类对比

不同的 TCP 类适用于不同场景：

| 类 | 适用场景 | 特点 |
|------|------|------|
| **TcpServer** | 服务端，接受多个客户端连接 | 封装 Acceptor + 多个 Connection，提供按 Token 管理客户端的接口 |
| **TcpAcceptor** | 服务端，仅接受连接 | 只负责接受连接，得到 TcpConnection 后自行管理 |
| **TcpConnector** | 客户端，带自动重连 | 支持重连策略、尝试次数限制 |
| **TcpClient** | 客户端，完整封装 | 封装 Connector + Connection，提供 ByteStream 接口 |

### TcpServer — TCP 服务端

| 方法 | 说明 |
|------|------|
| `TcpServer(loop)` | 构造 |
| `initialize(bind_addr, backlog)` | 初始化绑定地址 |
| `setTlsConfig(config)` | 设置 TLS 配置（必须在 initialize 之前调用） |
| `setConnectedCallback(cb)` | 设置新连接回调 |
| `setDisconnectedCallback(cb)` | 设置断开回调 |
| `setReceiveCallback(cb, threshold)` | 设置接收回调与数据阈值 |
| `setSendCompleteCallback(cb)` | 设置发送完成回调 |
| `start()` | 启动服务 |
| `send(client, data, size)` | 向指定客户端发送数据 |
| `disconnect(client)` | 断开指定客户端 |
| `stop()` | 停止服务 |
| `cleanup()` | 清理资源 |

### TcpClient — TCP 客户端

| 方法 | 说明 |
|------|------|
| `TcpClient(loop)` | 构造 |
| `initialize(server_addr)` | 初始化服务端地址 |
| `setConnectedCallback(cb)` | 设置连接成功回调 |
| `setDisconnectedCallback(cb)` | 设置断开回调 |
| `setAutoReconnect(enable)` | 设置自动重连 |
| `setTlsConfig(config)` | 设置 TLS 配置（必须在 initialize 之前调用） |
| `start()` | 开始连接 |
| `send(data, size)` | 发送数据（ByteStream 接口） |
| `bind(receiver)` | 绑定接收端（流水线模式） |
| `stop()` | 停止/断开连接 |
| `cleanup()` | 清理 |

### UdpSocket — UDP 奆接字

| 方法 | 说明 |
|------|------|
| `UdpSocket(loop, broadcast)` | 构造，指定是否启用广播 |
| `bind(addr)` | 绑定地址 |
| `connect(addr)` | 连接目标地址 |
| `setRecvCallback(cb)` | 设置接收回调 `(data, size, from_addr)` |
| `send(data, size, to_addr)` | 发送数据到指定地址 |
| `send(data, size)` | 发送数据（需先 connect） |
| `enable()` / `disable()` | 启用/停用接收 |

> **注意**：`bind()` 与 `connect()` 不能一起使用。

### Uart — 串口通信

| 方法 | 说明 |
|------|------|
| `Uart(loop)` | 构造 |
| `initialize(dev, mode_str)` | 初始化，dev 为设备路径，如 "/dev/ttyS0"，mode_str 如 "115200 8n1" |
| `initialize(dev, mode)` | 初始化，使用 Mode 结构体 |
| `send(data, size)` | 发送数据（ByteStream 接口） |
| `bind(receiver)` | 绑定接收端 |
| `enable()` / `disable()` | 启用/停用 |

Mode 结构体字段：`baudrate`(默认115200)、`data_bit`(k8bits)、`parity`(kNoEnd)、`stop_bit`(k1bits)

### SockAddr — 地址封装

```cpp
//! 从字符串创建地址
SockAddr addr = SockAddr::FromString("127.0.0.1:12345");
SockAddr addr = SockAddr::FromString("0.0.0.0:80");
```

### ByteStream — 字节流抽象

ByteStream 是一个统一的流接口，TcpClient、Uart 都实现了它。支持将两个 ByteStream 绑定形成数据流水线：

```cpp
//! 将 UART 与 TCP Client 绑定，串口数据直接转发到 TCP
uart->bind(tcp_client);
tcp_client->bind(uart);
```

## 使用示例

### TCP Echo 服务端

> 完整示例见 `examples/network/tcp_server/tcp_echo/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/network/tcp_server.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;

int main() {
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    TcpServer srv(sp_loop);
    srv.initialize(SockAddr::FromString("127.0.0.1:12345"), 2);

    //! 收到数据后原样回发（Echo）
    srv.setReceiveCallback(
        [&srv] (const TcpServer::ConnToken &client, Buffer &buff) {
            srv.send(client, buff.readableBegin(), buff.readableSize());
            buff.hasReadAll();
        }, 0
    );

    srv.start();

    //! 监听退出信号
    auto sp_sig = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_sig] { delete sp_sig; });
    sp_sig->initialize(SIGINT, Event::Mode::kOneshot);
    sp_sig->enable();
    sp_sig->setCallback([&] (int) { srv.stop(); sp_loop->exitLoop(); });

    sp_loop->runLoop();
    srv.cleanup();

    LogOutput_Disable();
    return 0;
}
```

### TCP 客户端

> 完整示例见 `examples/network/tcp_client/tcp_echo/`

```cpp
TcpClient client(sp_loop);
client.initialize(SockAddr::FromString("127.0.0.1:12345"));

client.setReceiveCallback(
    [] (Buffer &buff) {
        LogInfo("received: %.*s", (int)buff.readableSize(), (char*)buff.readableBegin());
        buff.hasReadAll();
    }, 0
);

client.start();
client.send("hello", 5);
```

### UDP Ping-Pong

> 完整示例见 `examples/network/udp_socket/ping_pong/`

```cpp
UdpSocket udp(sp_loop);
udp.bind(SockAddr::FromString("0.0.0.0:12345"));

udp.setRecvCallback(
    [&udp] (const void *data, size_t size, const SockAddr &from) {
        LogInfo("recv from %s", from.toString().c_str());
        udp.send("pong", 4, from);  //! 回复给发送方
    }
);
udp.enable();
```

### 串口通信

> 完整示例见 `examples/network/uart/uart_tool/`

```cpp
Uart uart(sp_loop);
uart.initialize("/dev/ttyS0", "115200 8n1");  //! 115200波特率，8数据位，无校验，1停止位

uart.setReceiveCallback(
    [] (Buffer &buff) {
        LogInfo("uart recv: %.*s", (int)buff.readableSize(), (char*)buff.readableBegin());
        buff.hasReadAll();
    }, 0
);

uart.enable();
uart.send("AT\r\n", 4);
```

### UART 转 TCP 桥接

> 完整示例见 `examples/network/uart/uart_to_uart/`

```cpp
//! 双向绑定，串口数据自动转发到 TCP，反之亦然
uart->bind(tcp_client);
tcp_client->bind(uart);
```

## TLS（SSL/TLS 加密通信）

network 模块通过可选的 `network_tls` 模块支持 TLS 加密通信。TcpServer 和 TcpClient 均可在 `initialize()` 之前调用 `setTlsConfig()` 将普通 TCP 升级为 TLS。TLS 实现基于 OpenSSL，支持 TLS 1.2 及以上版本。

### TlsConfig — TLS 配置结构体

```cpp
#include <tbox/network/tls_config.h>

struct TlsConfig {
    //! CA 证书（用于验证对端）
    std::string ca_file;        //!< CA 证书文件路径（如 "/etc/ssl/certs/ca-bundle.crt")
    std::string ca_path;        //!< CA 证书目录路径（如 "/etc/ssl/certs/")

    bool verify_peer = true;    //!< 是否验证对端证书
    int  verify_depth = 1;      //!< 证书链验证深度

    //! 本端证书和私钥
    std::string cert_file;      //!< 本端证书文件
    std::string key_file;       //!< 本端私钥文件

    //! Client SNI 配置
    std::string hostname;       //!< 用于 SNI (Server Name Indication) 的主机名

    bool isValid() const;       //!< 检查配置是否有效
};
```

**关于 `ca_file` / `ca_path` 的要点：**

- 它们是**可选的**，不需要必须设置。
- 当 `verify_peer=true` 但未指定 `ca_file`/`ca_path` 时：
  - **Client** 自动使用系统默认 CA 证书（调用 `SSL_CTX_set_default_verify_paths`），如 Linux 上的 `/etc/ssl/certs/`。这是最常见的使用场景。
  - **Server** 跳过客户端证书验证（适用于普通 TLS，非 mTLS）。
- 需要指定时，只需其中一个即可——`ca_file` 或 `ca_path`，不需要两者都设。OpenSSL 接受单独指定。
- `cert_file` 和 `key_file` 必须同时设置或同时为空，不能只设其中一个。

### TLS 工作原理

TLS 功能采用**弱符号插件**机制：

1. `network` 模块定义了一个弱符号的 `CreateTlsFactory()`，返回 `nullptr`。
2. `network_tls` 模块提供强符号实现，创建 `TcpTlsFactory`。
3. 如果应用链接了 `libtbox_network_tls`，TLS 功能可用；否则 `setTlsConfig()` 返回 `false` 并打印警告。

### TLS Echo 服务端

> 完整示例见 `examples/network/tcp_server/tls_echo_server/`

```cpp
TcpServer server(sp_loop);

//! 设置 TLS 配置（必须在 initialize 之前调用）
TlsConfig tls_config;
tls_config.cert_file = "server.crt";   //! 服务端必须设置证书和密钥
tls_config.key_file  = "server.key";
tls_config.verify_peer = false;         //! 不验证客户端证书（非 mTLS）
if (!server.setTlsConfig(tls_config)) {
    LogErr("TLS 不可用，需要链接 network_tls 模块");
    return;
}

server.initialize(SockAddr::FromString("0.0.0.0:12345"), 2);
server.start();
```

### TLS 客户端（使用自定义 CA 验证服务端）

> 完整示例见 `examples/network/tcp_client/tls_echo_client/`

```cpp
TcpClient client(sp_loop);

//! 使用自定义 CA 证书验证服务端
TlsConfig tls_config;
tls_config.ca_file = "server.crt";      //! 自定义 CA 证书
tls_config.verify_peer = true;          //! 验证服务端证书
tls_config.hostname = "myserver";       //! SNI 主机名
client.setTlsConfig(tls_config);

client.initialize(SockAddr::FromString("127.0.0.1:12345"));
client.start();
```

### TLS 客户端（使用系统默认 CA）

```cpp
TcpClient client(sp_loop);

//! 使用系统默认 CA 证书（如 /etc/ssl/certs/）
//! 不需要指定 ca_file 或 ca_path
TlsConfig tls_config;
tls_config.verify_peer = true;          //! 用系统 CA 验证服务端证书
tls_config.hostname = "example.com";    //! SNI 主机名
client.setTlsConfig(tls_config);

client.initialize(SockAddr::FromString("example.com:443"));
client.start();
```

### TLS 客户端（跳过验证，类似 curl -k）

```cpp
TcpClient client(sp_loop);

//! 跳过服务端证书验证（不安全，仅用于测试）
TlsConfig tls_config;
tls_config.verify_peer = false;
tls_config.hostname = "127.0.0.1";
client.setTlsConfig(tls_config);

client.initialize(SockAddr::FromString("127.0.0.1:12345"));
client.start();
```

### mTLS（双向 TLS — 双方都验证证书）

```cpp
//! 服务端：验证客户端证书
TlsConfig server_config;
server_config.cert_file = "server.crt";
server_config.key_file  = "server.key";
server_config.ca_file   = "client-ca.crt";  //! 签发客户端证书的 CA
server_config.verify_peer = true;            //! 验证客户端证书
server.setTlsConfig(server_config);

//! 客户端：验证服务端并向服务端出示自己的证书
TlsConfig client_config;
client_config.cert_file = "client.crt";      //! 向服务端出示客户端证书
client_config.key_file  = "client.key";
client_config.ca_file   = "server-ca.crt";   //! 签发服务端证书的 CA
client_config.verify_peer = true;
client_config.hostname   = "myserver";
client.setTlsConfig(client_config);
```

## 常见场景

1. **Echo 服务**：TcpServer 接收数据后原样回发
2. **命令行客户端**：TcpClient + StdioStream 实现交互式 TCP 客户端
3. **UART 桥接**：ByteStream bind 将串口数据转发到 TCP
4. **UDP 通信**：UdpSocket 实现广播或点对点 UDP
5. **自动重连客户端**：TcpClient + setAutoReconnect 实现断线自动重连
6. **TLS 服务端**：TcpServer + setTlsConfig 实现加密通信
7. **TLS 客户端（系统 CA）**：TcpClient + TlsConfig(verify_peer=true) 使用系统默认 CA 验证服务端
8. **mTLS 双向认证**：双方均设置 verify_peer=true + ca_file + cert_file/key_file

## 注意事项

1. **数据阈值 (threshold)**：`setReceiveCallback(cb, threshold)` 中 threshold 指定触发回调的最小数据量，0 表示收到任意数据即触发
2. **Buffer 的 hasReadAll**：回调中使用 `buff.hasReadAll()` 标记已读完数据，否则下次回调会重复收到旧数据
3. **TcpServer ConnToken**：通过 Token 标识客户端，Token 在连接断开后失效
4. **TcpClient 断线重连**：`setAutoReconnect(true)` 启用后，断线会自动尝试重连
5. **UDP bind vs connect**：bind 与 connect 不能一起使用，如需指定目标地址请在 send() 中传入
6. **TLS setTlsConfig**：必须在 `initialize()` 之前调用，且需要链接 `network_tls` 模块
7. **TLS ca_file/ca_path**：可选；verify_peer=true 但未指定时，客户端使用系统默认 CA；指定时只需其中一个
8. **TLS cert_file/key_file**：必须同时设置或同时为空，不能只设其中一个

## 相关模块

- **event**：基于 FdEvent 实现 socket 事件监听
- **http**：基于 TcpServer/TcpAcceptor 实现 HTTP 服务
- **mqtt**：基于 TcpConnection 实现 MQTT 协议
- **network_tls**：可选模块，基于 OpenSSL 为 TcpServer/TcpClient 提供 TLS 实现
- **base**：提供 Buffer（即 util::Buffer）、ScopeExit 等基础设施
