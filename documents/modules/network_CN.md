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

## 常见场景

1. **Echo 服务**：TcpServer 接收数据后原样回发
2. **命令行客户端**：TcpClient + StdioStream 实现交互式 TCP 客户端
3. **UART 桥接**：ByteStream bind 将串口数据转发到 TCP
4. **UDP 通信**：UdpSocket 实现广播或点对点 UDP
5. **自动重连客户端**：TcpClient + setAutoReconnect 实现断线自动重连

## 注意事项

1. **数据阈值 (threshold)**：`setReceiveCallback(cb, threshold)` 中 threshold 指定触发回调的最小数据量，0 表示收到任意数据即触发
2. **Buffer 的 hasReadAll**：回调中使用 `buff.hasReadAll()` 标记已读完数据，否则下次回调会重复收到旧数据
3. **TcpServer ConnToken**：通过 Token 标识客户端，Token 在连接断开后失效
4. **TcpClient 断线重连**：`setAutoReconnect(true)` 启用后，断线会自动尝试重连
5. **UDP bind vs connect**：bind 与 connect 不能一起使用，如需指定目标地址请在 send() 中传入

## 相关模块

- **event**：基于 FdEvent 实现 socket 事件监听
- **http**：基于 TcpServer/TcpAcceptor 实现 HTTP 服务
- **mqtt**：基于 TcpConnection 实现 MQTT 协议
- **base**：提供 Buffer（即 util::Buffer）、ScopeExit 等基础设施
