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
#include <tbox/network/tls_config.h>      //! TLS configuration
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
| `setTlsConfig(config)` | Set TLS config (must call before initialize) |
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
| `setTlsConfig(config)` | Set TLS config (must call before initialize) |
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

## TLS (SSL/TLS Encrypted Communication)

The network module supports TLS encryption through the optional `network_tls` module. Both TcpServer and TcpClient can be upgraded from plain TCP to TLS by calling `setTlsConfig()` before `initialize()`. The TLS implementation uses OpenSSL and supports TLS 1.2+.

### TlsConfig — TLS Configuration

```cpp
#include <tbox/network/tls_config.h>

struct TlsConfig {
    //! CA certificates (for verifying the peer)
    std::string ca_file;        //!< CA certificate file path (e.g. "/etc/ssl/certs/ca-bundle.crt")
    std::string ca_path;        //!< CA certificate directory path (e.g. "/etc/ssl/certs/")

    bool verify_peer = true;    //!< Whether to verify the peer's certificate
    int  verify_depth = 1;      //!< Certificate chain verification depth

    //! Local certificate and private key
    std::string cert_file;      //!< Local certificate file
    std::string key_file;       //!< Local private key file

    //! Client SNI
    std::string hostname;       //!< Hostname for SNI (Server Name Indication)

    bool isValid() const;       //!< Check if the configuration is valid
};
```

**Key points about `ca_file` / `ca_path`:**

- They are **optional** — you don't need to specify either one.
- When `verify_peer=true` but no `ca_file`/`ca_path` is provided:
  - **Client** automatically uses the system default CA certificates (`SSL_CTX_set_default_verify_paths`), such as `/etc/ssl/certs/` on Linux. This is the most common usage scenario.
  - **Server** skips client certificate verification (suitable for plain TLS without mTLS).
- When you do specify them, only one is required — `ca_file` or `ca_path`, not both. OpenSSL accepts either.
- `cert_file` and `key_file` must always be specified together (both set or both empty).

### How TLS Works

The TLS feature uses a **weak-symbol plugin** mechanism:

1. The `network` module defines a weak `CreateTlsFactory()` that returns `nullptr`.
2. The `network_tls` module provides a strong implementation that creates a `TcpTlsFactory`.
3. If your application links `libtbox_network_tls`, TLS is enabled; otherwise, `setTlsConfig()` returns `false` and logs a warning.

### TLS Echo Server

> Full example at `examples/network/tcp_server/tls_echo_server/`

```cpp
TcpServer server(sp_loop);

//! Set TLS config (must call before initialize)
TlsConfig tls_config;
tls_config.cert_file = "server.crt";   //! Server must have cert + key
tls_config.key_file  = "server.key";
tls_config.verify_peer = false;         //! Don't verify client cert (not mTLS)
if (!server.setTlsConfig(tls_config)) {
    LogErr("TLS not available, need network_tls module");
    return;
}

server.initialize(SockAddr::FromString("0.0.0.0:12345"), 2);
server.start();
```

### TLS Client (Verify Server with Custom CA)

> Full example at `examples/network/tcp_client/tls_echo_client/`

```cpp
TcpClient client(sp_loop);

//! Verify server certificate using a custom CA file
TlsConfig tls_config;
tls_config.ca_file = "server.crt";      //! Custom CA certificate
tls_config.verify_peer = true;          //! Verify server cert
tls_config.hostname = "myserver";       //! SNI hostname
client.setTlsConfig(tls_config);

client.initialize(SockAddr::FromString("127.0.0.1:12345"));
client.start();
```

### TLS Client (Use System Default CA)

```cpp
TcpClient client(sp_loop);

//! Use system default CA certificates (like /etc/ssl/certs/)
//! No need to specify ca_file or ca_path
TlsConfig tls_config;
tls_config.verify_peer = true;          //! Verify server cert with system CA
tls_config.hostname = "example.com";    //! SNI hostname
client.setTlsConfig(tls_config);

client.initialize(SockAddr::FromString("example.com:443"));
client.start();
```

### TLS Client (Skip Verification, like curl -k)

```cpp
TcpClient client(sp_loop);

//! Skip server certificate verification (insecure, for testing only)
TlsConfig tls_config;
tls_config.verify_peer = false;
tls_config.hostname = "127.0.0.1";
client.setTlsConfig(tls_config);

client.initialize(SockAddr::FromString("127.0.0.1:12345"));
client.start();
```

### mTLS (Mutual TLS — Both Sides Verify)

```cpp
//! Server side: verify client certificate
TlsConfig server_config;
server_config.cert_file = "server.crt";
server_config.key_file  = "server.key";
server_config.ca_file   = "client-ca.crt";  //! CA that signed client certs
server_config.verify_peer = true;            //! Verify client cert
server.setTlsConfig(server_config);

//! Client side: verify server and present own cert
TlsConfig client_config;
client_config.cert_file = "client.crt";      //! Present client cert to server
client_config.key_file  = "client.key";
client_config.ca_file   = "server-ca.crt";   //! CA that signed server certs
client_config.verify_peer = true;
client_config.hostname   = "myserver";
client.setTlsConfig(client_config);
```

## Common Scenarios

1. **Echo Service**: TcpServer receives data and sends it back as-is
2. **Interactive Client**: TcpClient + StdioStream implements an interactive TCP client
3. **UART Bridge**: ByteStream bind forwards serial data to TCP
4. **UDP Communication**: UdpSocket implements broadcast or point-to-point UDP
5. **Auto-reconnect Client**: TcpClient + setAutoReconnect implements automatic reconnect on disconnection
6. **TLS Server**: TcpServer + setTlsConfig enables encrypted communication
7. **TLS Client with System CA**: TcpClient + TlsConfig(verify_peer=true) verifies server using system CA store
8. **mTLS**: Both sides set verify_peer=true + ca_file + cert_file/key_file for mutual authentication

## Important Notes

1. **Data threshold**: In `setReceiveCallback(cb, threshold)`, threshold specifies the minimum data amount to trigger the callback; 0 means any received data triggers it immediately
2. **Buffer hasReadAll**: In callbacks, use `buff.hasReadAll()` to mark that all data has been read; otherwise, old data will be received again in the next callback
3. **TcpServer ConnToken**: Clients are identified by Token; the Token becomes invalid after the connection is disconnected
4. **TcpClient auto-reconnect**: After enabling `setAutoReconnect(true)`, disconnection will automatically attempt to reconnect
5. **UDP bind vs connect**: bind and connect cannot be used together; if you need to specify a target address, pass it in send()
6. **TLS setTlsConfig**: Must be called before `initialize()`, and requires linking the `network_tls` module
7. **TLS ca_file/ca_path**: Optional; when verify_peer=true without specifying them, client uses system default CA; only one is needed when you do specify them
8. **TLS cert_file/key_file**: Must always be specified together — both set or both empty

## Related Modules

- **event**: Implements socket event listening based on FdEvent
- **http**: Implements HTTP service based on TcpServer/TcpAcceptor
- **mqtt**: Implements MQTT protocol based on TcpConnection
- **network_tls**: Optional module providing OpenSSL-based TLS implementation for TcpServer/TcpClient
- **base**: Provides foundational infrastructure such as Buffer (i.e., util::Buffer), ScopeExit, etc.
