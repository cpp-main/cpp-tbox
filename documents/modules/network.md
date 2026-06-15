# Network Communication Module (network)

## What is it?

The network module provides TCP/UDP/UART communication capabilities based on the event module, including server-side (TcpServer/TcpAcceptor), client-side (TcpClient/TcpConnector), UDP communication (UdpSocket), serial communication (Uart), and byte stream abstraction (ByteStream).

## Why do you need it?

In service-oriented programs, network communication is the most fundamental requirement. However, traditional socket programming requires handling a large amount of detail: fd management, event listening, data buffering, connection management, and more. The network module encapsulates all of these into object-oriented interfaces that integrate seamlessly with the event loop, allowing developers to focus solely on business logic.

## Header Files

```cpp
#include <tbox/network/tcp_server.h>      //! TCP server
#include <tbox/network/tcp_acceptor.h>    //! TCP connection acceptor
#include <tbox/network/tcp_connector.h>   //! TCP connector (with auto-reconnect)
#include <tbox/network/tcp_client.h>      //! TCP client (encapsulates connection + communication)
#include <tbox/network/tcp_connection.h>  //! TCP connection (low-level)
#include <tbox/network/udp_socket.h>      //! UDP socket
#include <tbox/network/uart.h>            //! Serial communication
#include <tbox/network/byte_stream.h>     //! Byte stream abstract interface
#include <tbox/network/buffered_fd.h>     //! Buffered fd
#include <tbox/network/sockaddr.h>        //! Address wrapper
#include <tbox/network/socket_fd.h>       //! Socket fd
#include <tbox/network/ip_address.h>      //! IP address
#include <tbox/network/dns_request.h>     //! DNS request
#include <tbox/network/domain_name.h>     //! Domain name resolution
#include <tbox/network/net_if.h>          //! Network interface
#include <tbox/network/stdio_stream.h>    //! Standard I/O stream
```

## Core Classes and Interfaces

### TCP Class Comparison

Different TCP classes are suited for different scenarios:

| Class | Use Case | Characteristics |
|------|------|------|
| **TcpServer** | Server side, accepting multiple client connections | Encapsulates Acceptor + multiple Connections, provides client management interface by Token |
| **TcpAcceptor** | Server side, only accepting connections | Only responsible for accepting connections; after obtaining a TcpConnection, you manage it yourself |
| **TcpConnector** | Client side, with auto-reconnect | Supports reconnect strategies and attempt count limits |
| **TcpClient** | Client side, full encapsulation | Encapsulates Connector + Connection, provides ByteStream interface |

### TcpServer — TCP Server

| Method | Description |
|------|------|
| `TcpServer(loop)` | Constructor |
| `initialize(bind_addr, backlog)` | Initialize bind address |
| `setConnectedCallback(cb)` | Set new connection callback |
| `setDisconnectedCallback(cb)` | Set disconnect callback |
| `setReceiveCallback(cb, threshold)` | Set receive callback and data threshold |
| `setSendCompleteCallback(cb)` | Set send complete callback |
| `start()` | Start service |
| `send(client, data, size)` | Send data to specified client |
| `disconnect(client)` | Disconnect specified client |
| `stop()` | Stop service |
| `cleanup()` | Clean up resources |

### TcpClient — TCP Client

| Method | Description |
|------|------|
| `TcpClient(loop)` | Constructor |
| `initialize(server_addr)` | Initialize server address |
| `setConnectedCallback(cb)` | Set connection success callback |
| `setDisconnectedCallback(cb)` | Set disconnect callback |
| `setAutoReconnect(enable)` | Set auto-reconnect |
| `start()` | Start connection |
| `send(data, size)` | Send data (ByteStream interface) |
| `bind(receiver)` | Bind receiver (pipeline mode) |
| `stop()` | Stop/disconnect |
| `cleanup()` | Clean up |

### UdpSocket — UDP Socket

| Method | Description |
|------|------|
| `UdpSocket(loop, broadcast)` | Constructor, specify whether to enable broadcast |
| `bind(addr)` | Bind address |
| `connect(addr)` | Connect to target address |
| `setRecvCallback(cb)` | Set receive callback `(data, size, from_addr)` |
| `send(data, size, to_addr)` | Send data to specified address |
| `send(data, size)` | Send data (requires prior connect) |
| `enable()` / `disable()` | Enable/disable receiving |

> **Note**: `bind()` and `connect()` cannot be used together.

### Uart — Serial Communication

| Method | Description |
|------|------|
| `Uart(loop)` | Constructor |
| `initialize(dev, mode_str)` | Initialize, dev is the device path such as "/dev/ttyS0", mode_str such as "115200 8n1" |
| `initialize(dev, mode)` | Initialize, using Mode struct |
| `send(data, size)` | Send data (ByteStream interface) |
| `bind(receiver)` | Bind receiver |
| `enable()` / `disable()` | Enable/disable |

Mode struct fields: `baudrate` (default 115200), `data_bit` (k8bits), `parity` (kNoEnd), `stop_bit` (k1bits)

### SockAddr — Address Wrapper

```cpp
//! Create address from string
SockAddr addr = SockAddr::FromString("127.0.0.1:12345");
SockAddr addr = SockAddr::FromString("0.0.0.0:80");
```

### ByteStream — Byte Stream Abstraction

ByteStream is a unified stream interface that both TcpClient and Uart implement. It supports binding two ByteStreams together to form a data pipeline:

```cpp
//! Bind UART with TCP Client, serial data is forwarded directly to TCP
uart->bind(tcp_client);
tcp_client->bind(uart);
```

## Usage Examples

### TCP Echo Server

> Full example at `examples/network/tcp_server/tcp_echo/`

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

    //! Echo: send received data back as-is
    srv.setReceiveCallback(
        [&srv] (const TcpServer::ConnToken &client, Buffer &buff) {
            srv.send(client, buff.readableBegin(), buff.readableSize());
            buff.hasReadAll();
        }, 0
    );

    srv.start();

    //! Listen for exit signal
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

### TCP Client

> Full example at `examples/network/tcp_client/tcp_echo/`

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

> Full example at `examples/network/udp_socket/ping_pong/`

```cpp
UdpSocket udp(sp_loop);
udp.bind(SockAddr::FromString("0.0.0.0:12345"));

udp.setRecvCallback(
    [&udp] (const void *data, size_t size, const SockAddr &from) {
        LogInfo("recv from %s", from.toString().c_str());
        udp.send("pong", 4, from);  //! Reply to sender
    }
);
udp.enable();
```

### Serial Communication

> Full example at `examples/network/uart/uart_tool/`

```cpp
Uart uart(sp_loop);
uart.initialize("/dev/ttyS0", "115200 8n1");  //! 115200 baud rate, 8 data bits, no parity, 1 stop bit

uart.setReceiveCallback(
    [] (Buffer &buff) {
        LogInfo("uart recv: %.*s", (int)buff.readableSize(), (char*)buff.readableBegin());
        buff.hasReadAll();
    }, 0
);

uart.enable();
uart.send("AT\r\n", 4);
```

### UART to TCP Bridge

> Full example at `examples/network/uart/uart_to_uart/`

```cpp
//! Bidirectional binding: serial data is automatically forwarded to TCP, and vice versa
uart->bind(tcp_client);
tcp_client->bind(uart);
```

## Common Scenarios

1. **Echo Service**: TcpServer receives data and sends it back as-is
2. **Interactive Client**: TcpClient + StdioStream implements an interactive TCP client
3. **UART Bridge**: ByteStream bind forwards serial data to TCP
4. **UDP Communication**: UdpSocket implements broadcast or point-to-point UDP
5. **Auto-reconnect Client**: TcpClient + setAutoReconnect implements automatic reconnect on disconnection

## Important Notes

1. **Data threshold**: In `setReceiveCallback(cb, threshold)`, threshold specifies the minimum data amount to trigger the callback; 0 means any received data triggers it immediately
2. **Buffer hasReadAll**: In callbacks, use `buff.hasReadAll()` to mark that all data has been read; otherwise, old data will be received again in the next callback
3. **TcpServer ConnToken**: Clients are identified by Token; the Token becomes invalid after the connection is disconnected
4. **TcpClient auto-reconnect**: After enabling `setAutoReconnect(true)`, disconnection will automatically attempt to reconnect
5. **UDP bind vs connect**: bind and connect cannot be used together; if you need to specify a target address, pass it in send()

## Related Modules

- **event**: Implements socket event listening based on FdEvent
- **http**: Implements HTTP service based on TcpServer/TcpAcceptor
- **mqtt**: Implements MQTT protocol based on TcpConnection
- **base**: Provides foundational infrastructure such as Buffer (i.e., util::Buffer), ScopeExit, etc.
