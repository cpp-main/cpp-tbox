# WebSocket Service Module (websocket)

## What is it?

The websocket module provides WebSocket server and client implementations based on the HTTP server module. It follows the RFC 6455 specification and integrates with the HTTP middleware pattern — the WsServer itself is an HTTP middleware that detects WebSocket upgrade requests and manages upgraded connections.

On the client side, the `Client` class establishes a TCP connection to a WebSocket server, performs the HTTP Upgrade handshake, and then enters WebSocket frame communication mode. It supports auto-reconnect with configurable delay strategies.

## Why do you need it?

In service programs that already expose HTTP APIs, you may also need real-time bidirectional communication — for example: pushing live updates to browsers, chat rooms, IoT device status streaming, or binary data echo services. The websocket module allows you to add WebSocket capability alongside your existing HTTP server without running a separate service.

On the client side, C++ programs may need to connect to WebSocket servers to receive real-time push data or send commands. The `Client` class provides an asynchronous WebSocket client with auto-reconnect, Ping/Pong heartbeat, and Close frame handling.

## Header Files

```cpp
#include <tbox/websocket/ws_frame.h>               //! WebSocket frame definition
#include <tbox/websocket/ws_frame_parser.h>        //! Frame parser (incremental)
#include <tbox/websocket/ws_frame_builder.h>       //! Frame builder (server/masked)
#include <tbox/websocket/server/ws_server.h>        //! WebSocket server
#include <tbox/websocket/server/ws_connection.h>    //! WebSocket connection (internal)
#include <tbox/websocket/client/client.h>           //! WebSocket client
```

## Core Classes and Interfaces

### WsServer — WebSocket Server

WsServer runs on top of an HTTP server as a middleware. It detects WebSocket upgrade requests, validates the handshake, and creates WsConnection objects for each upgraded connection. All client operations use `ConnToken` (a cabinet::Token) instead of raw pointers.

| Method | Description |
|------|------|
| `WsServer(loop)` | Constructor |
| `initialize(http_server, url_path)` | Initialize: associate with an HTTP server; `url_path` controls URL matching |
| `start()` | Start (registers as HTTP middleware) |
| `stop()` | Stop (unregisters middleware, closes all connections) |
| `cleanup()` | Cleanup (inverse of initialize) |
| `state()` | Get current state (None/Inited/Running) |
| `send(client, text)` | Send text frame to a client |
| `send(client, data, len)` | Send binary frame to a client (raw pointer version) |
| `sendBinary(client, data)` | Send binary frame to a client (vector version) |
| `close(client, code, reason)` | Close a client connection (sends Close frame) |
| `ping(client, data)` | Send Ping frame to a client |
| `pong(client, data)` | Send Pong frame to a client |
| `isClientValid(client)` | Check if a client connection is still valid |
| `peerAddr(client)` | Get client address (IP:port) |
| `getUrl(client)` | Get the URL path the client connected to |
| `setContext(client, ctx, deleter)` | Set context data for a client connection |
| `getContext(client)` | Get context data for a client connection |
| `setConnectedCallback(cb)` | Set callback: new client connected |
| `setDisconnectedCallback(cb)` | Set callback: client disconnected |
| `setMessageCallback(cb)` | Set callback: client sent a message |
| `setErrorCallback(cb)` | Set callback: client connection error |
| `IsWsUpgradeRequest(req)` | Static: check if an HTTP request is a valid WebSocket upgrade |
| `ComputeWsAcceptKey(key)` | Static: compute Sec-WebSocket-Accept value |

**URL path matching rules:**

| `url_path` value | Matching behavior |
|---|---|
| Ends with `/` (e.g. `/ws/`) | Prefix match — matches `/ws/aa`, `/ws/bb/cc` |
| Does not end with `/` (e.g. `/ws`) | Exact match — matches only `/ws` |
| Empty string `""` | Matches all WebSocket upgrade requests |

**State enum:**

| State | Description |
|-------|-------------|
| `kNone` | Not initialized |
| `kInited` | Initialized |
| `kRunning` | Running (middleware registered) |

**Callback signatures:**

```cpp
using ConnToken = cabinet::Token;

ConnectedCallback    = std::function<void(const ConnToken&)>;
DisconnectedCallback = std::function<void(const ConnToken&)>;
MessageCallback      = std::function<void(const ConnToken&, const WsFrame&)>;
ErrorCallback        = std::function<void(const ConnToken&)>;
```

### Client — WebSocket Client

The Client class connects to a WebSocket server via TcpConnector, performs the HTTP Upgrade handshake, and then enters WebSocket frame communication mode. All client-to-server frames are masked per RFC 6455. It supports auto-reconnect with configurable delay strategies.

| Method | Description |
|------|------|
| `Client(loop)` | Constructor |
| `initialize(server_addr, url_path)` | Initialize: set target server address and URL path |
| `start()` | Start connecting to server |
| `stop()` | Stop/disconnect |
| `cleanup()` | Cleanup (inverse of initialize) |
| `state()` | Get current state |
| `send(text)` | Send text frame |
| `send(data, len)` | Send binary frame (raw pointer version) |
| `sendBinary(data)` | Send binary frame (vector version) |
| `close(code, reason)` | Send Close frame and disconnect |
| `ping(data)` | Send Ping frame |
| `pong(data)` | Send Pong frame |
| `isExpired()` | Check if connection is expired |
| `peerAddr()` | Get server address |
| `setContext(ctx, deleter)` | Set context data |
| `getContext()` | Get context data |
| `setConnectedCallback(cb)` | Set callback: connected to server |
| `setDisconnectedCallback(cb)` | Set callback: disconnected from server |
| `setMessageCallback(cb)` | Set callback: received a message |
| `setErrorCallback(cb)` | Set callback: connection error |
| `setAutoReconnect(enable)` | Enable/disable auto-reconnect (default: enabled) |
| `setReconnectDelayCalcFunc(func)` | Set custom reconnect delay calculation |

**State enum:**

| State | Description |
|-------|-------------|
| `kNone` | Not initialized |
| `kInited` | Initialized |
| `kConnecting` | TCP connecting |
| `kHandshaking` | HTTP Upgrade handshake in progress |
| `kConnected` | WebSocket connected |

### WsFrame — WebSocket Frame

```cpp
struct WsFrame {
    enum class OpCode : uint8_t {
        kContinue = 0x0,   //! Continuation
        kText     = 0x1,   //! Text data
        kBinary   = 0x2,   //! Binary data
        kClose    = 0x8,   //! Close connection
        kPing     = 0x9,   //! Ping
        kPong     = 0xA,   //! Pong
    };

    OpCode  opcode;         //! Frame opcode
    bool    fin = true;     //! Is this the final frame?
    std::string payload;    //! Payload data

    bool isControlFrame() const;  //! Close/Ping/Pong are control frames
    uint16_t closeCode() const;   //! Extract close code from Close frame
    std::string closeReason() const; //! Extract close reason from Close frame
};
```

### WsFrameParser — Incremental Frame Parser

An incremental WebSocket frame parser suitable for event-driven scenarios. It parses data from a buffer step by step.

| Method | Description |
|------|------|
| `parse(data, size)` | Parse data, returns number of bytes consumed |
| `state()` | Get current parsing state |
| `getFrame()` | Get the parsed frame (only when state == kFinished) |
| `reset()` | Reset parser |

### WsFrameBuilder — Frame Builder

Static helper class for constructing WebSocket frames. Server frames are unmasked; client frames are masked per RFC 6455.

| Method | Description |
|------|------|
| `BuildTextFrame(text)` | Build text frame (server, unmasked) |
| `BuildBinaryFrame(data, len)` | Build binary frame (server, unmasked) |
| `BuildBinaryFrame(data)` | Build binary frame (server, vector version) |
| `BuildCloseFrame(code, reason)` | Build Close frame (server, unmasked) |
| `BuildPingFrame(data)` | Build Ping frame (server, unmasked) |
| `BuildPongFrame(data)` | Build Pong frame (server, unmasked) |
| `BuildFrame(opcode, fin, payload, len)` | Build generic frame (server, unmasked) |
| `BuildMaskedTextFrame(text)` | Build text frame (client, masked) |
| `BuildMaskedBinaryFrame(data, len)` | Build binary frame (client, masked) |
| `BuildMaskedBinaryFrame(data)` | Build binary frame (client, masked, vector) |
| `BuildMaskedCloseFrame(code, reason)` | Build Close frame (client, masked) |
| `BuildMaskedPingFrame(data)` | Build Ping frame (client, masked) |
| `BuildMaskedPongFrame(data)` | Build Pong frame (client, masked) |
| `BuildMaskedFrame(opcode, fin, payload, len, mask_key)` | Build generic frame (client, masked) |

## Usage Examples

### Server: Chat Room

> Full example in `examples/websocket/chat_server/`

Demonstrates multiple chat rooms mounted on the same HTTP server at different URL paths. Each ChatRoom contains a WsServer instance and manages WebSocket connections. The first text message from a client is treated as the username (login), and subsequent messages are broadcast to all logged-in users.

```cpp
#include <tbox/http/server/server.h>
#include <tbox/websocket/server/ws_server.h>

class ChatRoom {
  public:
    ChatRoom(event::Loop *wp_loop, const std::string &name)
      : ws_srv_(wp_loop)
    { }

    bool initialize(http::server::Server *http_srv, const std::string &url_path)
    {
        if (!ws_srv_.initialize(http_srv, url_path))
            return false;

        ws_srv_.setConnectedCallback([this](const WsServer::ConnToken &token) {
            onConnected(token);
        });
        ws_srv_.setDisconnectedCallback([this](const WsServer::ConnToken &token) {
            onDisconnected(token);
        });
        ws_srv_.setMessageCallback([this](const WsServer::ConnToken &token, const WsFrame &frame) {
            onMessage(token, frame);
        });

        return true;
    }

    bool start() { return ws_srv_.start(); }
    void stop()  { ws_srv_.stop(); }
    void cleanup() { ws_srv_.cleanup(); }

  private:
    void onMessage(const WsServer::ConnToken &token, const WsFrame &frame)
    {
        if (frame.opcode != WsFrame::OpCode::kText)
            return;

        //! First message is the username
        auto it = conn_to_name_.find(token);
        if (it == conn_to_name_.end()) {
            conn_to_name_[token] = frame.payload;
            broadcast(frame.payload + " online");
        } else {
            broadcast(it->second + ": " + frame.payload);
        }
    }

    void broadcast(const std::string &msg)
    {
        for (const auto &pair : conn_to_name_)
            ws_srv_.send(pair.first, msg);
    }

    WsServer ws_srv_;
    std::map<WsServer::ConnToken, std::string> conn_to_name_;
};

int main()
{
    auto sp_loop = Loop::New();

    //! Create HTTP server
    Server http_srv(sp_loop);
    http_srv.initialize(network::SockAddr::FromString("0.0.0.0:8080"), 1);

    //! Create two chat rooms at different URL paths
    ChatRoom chat_room_1(sp_loop, "Room1");
    ChatRoom chat_room_2(sp_loop, "Room2");

    chat_room_1.initialize(&http_srv, "/ws/chat-1");
    chat_room_2.initialize(&http_srv, "/ws/chat-2");

    //! Add HTTP homepage handler
    http_srv.use([&](ContextSptr ctx, const NextFunc &next) {
        if (ctx->req().url.path == "/") {
            ctx->res().status_code = StatusCode::k200_OK;
            ctx->res().body = "<h1>Chat Server</h1>";
            return;
        }
        next();
    });

    //! Start services
    http_srv.start();
    chat_room_1.start();
    chat_room_2.start();

    //! ... run loop, handle SIGINT, cleanup ...
}
```

### Server: Binary Echo

> Full example in `examples/websocket/echo_bin/`

Demonstrates binary WebSocket frame handling. The server echoes binary data back to the client and periodically pushes statistics frames (4-byte header "STAT" + JSON payload) using `sendBinary()` with `vector<uint8_t>`.

```cpp
class EchoService {
  public:
    EchoService(Loop *wp_loop)
      : ws_srv_(wp_loop)
      , stat_timer_(wp_loop->newTimerEvent())
    { }

    bool initialize(Server *http_srv, const std::string &url_path)
    {
        if (!ws_srv_.initialize(http_srv, url_path))
            return false;

        ws_srv_.setMessageCallback([this](const WsServer::ConnToken &token, const WsFrame &frame) {
            onMessage(token, frame);
        });

        //! Timer: push stats every 5 seconds
        stat_timer_->initialize(std::chrono::milliseconds(5000), Event::Mode::kPersist);
        stat_timer_->setCallback([this] { onStatTimer(); });

        return true;
    }

  private:
    void onMessage(const WsServer::ConnToken &token, const WsFrame &frame)
    {
        if (frame.opcode == WsFrame::OpCode::kBinary) {
            //! Echo binary data back
            ws_srv_.send(token, frame.payload.data(), frame.payload.size());
        } else if (frame.opcode == WsFrame::OpCode::kText) {
            ws_srv_.send(token, "This service only accepts binary frames");
        }
    }

    void onStatTimer()
    {
        //! Build binary statistics frame: "STAT" header + JSON
        std::vector<uint8_t> stat_data;
        stat_data.insert(stat_data.end(), kStatHeader, kStatHeader + 4);
        stat_data.insert(stat_data.end(), json.begin(), json.end());

        for (const auto &token : conns_)
            ws_srv_.sendBinary(token, stat_data);
    }
};
```

### Client: Chat Client

> Full example in `examples/websocket/chat_client/`

Demonstrates a WebSocket client connecting to a chat server, reading from stdin, and sending/receiving messages.

```cpp
#include <tbox/websocket/client/client.h>

int main()
{
    auto sp_loop = Loop::New();

    Client ws_client(sp_loop);
    ws_client.initialize(SockAddr::FromString("127.0.0.1:8080"), "/ws/chat-1");

    ws_client.setConnectedCallback([&] {
        std::cout << "Connected! Enter your username:" << std::endl;
        //! Enable stdin reading
        sp_stdin_event->enable();
    });

    ws_client.setMessageCallback([&](const WsFrame &frame) {
        if (frame.opcode == WsFrame::OpCode::kText) {
            std::cout << frame.payload << std::endl;
        }
    });

    //! Custom reconnect delay: exponential backoff
    ws_client.setReconnectDelayCalcFunc([](int fail_count) {
        return 1 << std::min(4, fail_count);
    });

    ws_client.start();

    //! ... run loop, handle SIGINT, cleanup ...
}
```

### Server: URL Path Matching

```cpp
//! Prefix match: matches /ws/anything
ws_srv.initialize(&http_srv, "/ws/");

//! Exact match: matches only /ws
ws_srv.initialize(&http_srv, "/ws");

//! Match all: matches any WebSocket upgrade request
ws_srv.initialize(&http_srv, "");
```

### Server: Context Data

Attach custom data to a client connection for per-client state tracking:

```cpp
ws_srv.setConnectedCallback([](const WsServer::ConnToken &token) {
    //! Attach a user session object
    auto session = new UserSession();
    ws_srv.setContext(token, session, [](void *p) { delete static_cast<UserSession*>(p); });
});

ws_srv.setMessageCallback([](const WsServer::ConnToken &token, const WsFrame &frame) {
    //! Retrieve the session
    auto session = static_cast<UserSession*>(ws_srv.getContext(token));
    if (session != nullptr) {
        //! ... process message using session data ...
    }
});
```

### Client: Custom Reconnect Delay

```cpp
//! Exponential backoff: 1s, 2s, 4s, 8s, ... max 16s
ws_client.setReconnectDelayCalcFunc([](int fail_count) {
    return 1 << std::min(4, fail_count);
});

//! Disable auto-reconnect
ws_client.setAutoReconnect(false);
```

## Common Scenarios

1. **Real-time push**: Mount WsServer on HTTP server, push live data to browser clients
2. **Chat room**: Multiple chat rooms on the same HTTP server at different URL paths
3. **Binary data streaming**: Echo binary frames, send structured binary data with headers
4. **IoT device communication**: Client connects to server, sends status updates and receives commands
5. **Server-to-client heartbeat**: Server sends Ping frames, client auto-replies Pong
6. **Client auto-reconnect**: Client reconnects with exponential backoff after disconnection
7. **Mixed HTTP + WebSocket**: HTTP serves REST APIs and static pages; WebSocket handles real-time communication

## Important Notes

1. **WsServer is an HTTP middleware**: It must be initialized with an `http::server::Server` and started before or after the HTTP server starts. WsServer registers itself as a middleware via `http_server->use()`.
2. **URL path matching**: Pay attention to whether `url_path` ends with `/` — it determines prefix matching vs. exact matching. An empty string matches all upgrade requests.
3. **ConnToken-based operations**: All client operations use `ConnToken` (cabinet::Token), not pointers. This ensures safe access even after the underlying connection is destroyed.
4. **Ping/Pong auto-reply**: Both WsServer and Client automatically reply Pong when receiving Ping frames.
5. **Close frame auto-reply**: Both sides automatically send a Close frame reply when receiving a Close frame, then wait for TCP disconnection.
6. **Client masking**: Per RFC 6455, all frames sent by the Client are masked. Server frames are unmasked.
7. **Client auto-reconnect**: Default is enabled. Disconnection triggers automatic reconnection after the configured delay. Customize delay via `setReconnectDelayCalcFunc()`.
8. **Client handshake**: The Client performs HTTP Upgrade handshake automatically. It validates the 101 response, Sec-WebSocket-Accept header, and Upgrade/Connection headers. Handshake failure triggers reconnection if auto-reconnect is enabled.
9. **Lifecycle order**: Must follow initialize → start → stop → cleanup for both WsServer and Client.
10. **Thread safety**: All callbacks run in the Loop thread. Cross-thread operations must use `runInLoop()`.
11. **Context data**: `setContext()/getContext()` on WsServer delegates to the underlying TcpConnection. Context data is accessible in callbacks but becomes `nullptr` after the connection is destroyed.

## Related Modules

- **http**: WsServer runs as an HTTP middleware on http::server::Server
- **event**: Server and Client run based on Loop
- **network**: Connection management via TcpConnector (client) and TcpConnection
- **crypto**: SHA1 calculation for Sec-WebSocket-Accept
- **base**: Provides Cabinet for connection lifetime management
