# WebSocket 服务模块 (websocket)

## 是什么？

websocket 模块提供了 WebSocket 服务器与客户端实现，遵循 RFC 6455 规范。服务端基于 HTTP 服务器中间件模式运行——WsServer 本身即为 HTTP 中间件，它检测 WebSocket 升级请求、完成握手、接管 TcpConnection 进入帧通信模式。

客户端侧，`Client` 类通过 TcpConnector 建立 TCP 连接，发送 HTTP Upgrade 握手请求，验证 101 响应后进入帧通信模式。所有客户端帧必须掩码（RFC 6455 Section 5.3），支持自动重连与可配置的重连延迟策略。

## 为什么需要它？

在已提供 HTTP API 的服务程序中，还需要实时双向通信的场景——例如：向浏览器推送实时数据、聊天室、IoT 设备状态流、二进制数据回传等。websocket 模块让 C++ 程序在现有 HTTP 服务器上叠加 WebSocket 能力，无需额外部署独立服务。

客户端侧，C++ 程序可能需要连接 WebSocket 服务器接收实时推送数据或发送指令。`Client` 类提供了异步 WebSocket 客户端，支持自动重连、Ping/Pong 心跳和 Close 帧处理。

## 头文件

```cpp
#include <tbox/websocket/ws_frame.h>               //! WebSocket 帧定义
#include <tbox/websocket/ws_frame_parser.h>        //! 帧解析器（增量式）
#include <tbox/websocket/ws_frame_builder.h>       //! 帧构建器（服务端/掩码）
#include <tbox/websocket/ws_compressor.h>          //! 压缩（RFC 7692）
#include <tbox/websocket/server/ws_server.h>        //! WebSocket 服务端
#include <tbox/websocket/server/ws_connection.h>    //! WebSocket 连接（内部类）
#include <tbox/websocket/client/ws_client.h>         //! WebSocket 客户端
```

## 核心类与接口

### WsServer — WebSocket 服务端

WsServer 运行在 HTTP 服务器之上，作为中间件存在。它检测 WebSocket 升级请求，验证握手参数，并为每个升级成功的连接创建 WsConnection 对象。所有客户端操作使用 `ConnToken`（cabinet::Token），而非原始指针。

| 方法 | 说明 |
|------|------|
| `WsServer(loop)` | 构造 |
| `initialize(http_server, url_path)` | 初始化：关联到 HTTP 服务器；`url_path` 控制URL匹配规则 |
| `start()` | 启动（注册为 HTTP 中间件） |
| `stop()` | 停止（反注册中间件，关闭所有连接） |
| `cleanup()` | 清理（与 initialize 逆操作） |
| `state()` | 获取当前状态 (None/Inited/Running) |
| `send(client, text)` | 向指定客户端发送文本帧 |
| `send(client, str)` | 向指定客户端发送文本帧（const char* 版本，不构造 std::string） |
| `send(client, data, len)` | 向指定客户端发送二进制帧（原始指针版本） |
| `send(client, data)` | 向指定客户端发送二进制帧（vector 版本） |
| `close(client, code, reason)` | 关闭指定客户端连接（发送 Close 帧） |
| `ping(client, data)` | 向指定客户端发送 Ping 帧 |
| `pong(client, data)` | 向指定客户端发送 Pong 帧 |
| `isClientValid(client)` | 检查客户端连接是否有效 |
| `peerAddr(client)` | 获取客户端地址（IP:端口） |
| `getUrl(client)` | 获取客户端连接的 URL 路径 |
| `setContext(client, ctx, deleter)` | 设置客户端连接的上下文数据 |
| `getContext(client)` | 获取客户端连接的上下文数据 |
| `setConnectedCallback(cb)` | 设置回调：新客户端连接 |
| `setDisconnectedCallback(cb)` | 设置回调：客户端断开 |
| `setTextMessageCallback(cb)` | 设置回调：收到完整文本消息（右值引用，分片数据缓存后统一解压再回调） |
| `setBinaryMessageCallback(cb)` | 设置回调：收到完整二进制消息（右值引用，分片数据缓存后统一解压再回调） |
| `setErrorCallback(cb)` | 设置回调：客户端连接出错 |
| `setCompressionEnable(enable)` | 启用/禁用压缩支持（必须在 initialize 之前调用） |
| `setFragmentSize(size)` | 设置发送分片大小（默认65535，0=不分片；必须在 initialize 之前调用） |
| `IsWsUpgradeRequest(req)` | 静态方法：检查 HTTP 请求是否为有效的 WebSocket 升级请求 |
| `ComputeWsAcceptKey(key)` | 静态方法：计算 Sec-WebSocket-Accept 响应值 |

**URL 路径匹配规则：**

| `url_path` 值 | 匹配行为 |
|---|---|
| 以 `/` 结尾（如 `/ws/`） | 前缀匹配——匹配 `/ws/aa`、`/ws/bb/cc` |
| 不以 `/` 结尾（如 `/ws`） | 全量匹配——仅匹配 `/ws` |
| 空字符串 `""` | 匹配所有 WebSocket 升级请求 |

**State 状态枚举：**

| 状态 | 说明 |
|------|------|
| `kNone` | 未初始化 |
| `kInited` | 已初始化 |
| `kRunning` | 运行中（中间件已注册） |

**回调签名：**

```cpp
using ConnToken = cabinet::Token;

ConnectedCallback     = std::function<void(const ConnToken&)>;
DisconnectedCallback  = std::function<void(const ConnToken&)>;
TextMessageCallback   = std::function<void(const ConnToken&, std::string &&)>;
BinaryMessageCallback = std::function<void(const ConnToken&, std::vector<uint8_t> &&)>;
ErrorCallback         = std::function<void(const ConnToken&)>;
```

> **注意：** `TextMessageCallback` 和 `BinaryMessageCallback` 使用右值引用提升效率。分片消息在内部缓存，只有接收完整（fin=true）并解压后才回调业务层，回调中永远不会收到部分分片数据。

### WsClient — WebSocket 客户端

WsClient 类通过 TcpConnector 建立 TCP 连接，发送 HTTP Upgrade 握手请求，验证 101 响应后进入 WebSocket 帧通信模式。所有客户端帧必须掩码（RFC 6455）。支持自动重连与可配置的重连延迟策略。分片消息在内部缓存，接收完整后再解压回调。

| 方法 | 说明 |
|------|------|
| `WsClient(loop)` | 构造 |
| `initialize(server_addr, url_path)` | 初始化：设置目标服务器地址与 URL 路径 |
| `start()` | 开始连接服务器 |
| `stop()` | 停止/断开连接 |
| `cleanup()` | 清理（与 initialize 逆操作） |
| `state()` | 获取当前状态 |
| `send(text)` | 发送文本帧 |
| `send(str)` | 发送文本帧（const char* 版本，不构造 std::string） |
| `send(data, len)` | 发送二进制帧（原始指针版本） |
| `send(data)` | 发送二进制帧（vector 版本） |
| `close(code, reason)` | 发送 Close 帧并断开连接 |
| `ping(data)` | 发送 Ping 帧 |
| `pong(data)` | 发送 Pong 帧 |
| `isExpired()` | 检查连接是否已失效 |
| `peerAddr()` | 获取服务器地址 |
| `setContext(ctx, deleter)` | 设置上下文数据 |
| `getContext()` | 获取上下文数据 |
| `setConnectedCallback(cb)` | 设置回调：连接成功 |
| `setDisconnectedCallback(cb)` | 设置回调：连接断开 |
| `setTextMessageCallback(cb)` | 设置回调：收到完整文本消息（右值引用） |
| `setBinaryMessageCallback(cb)` | 设置回调：收到完整二进制消息（右值引用） |
| `setErrorCallback(cb)` | 设置回调：连接出错 |
| `setAutoReconnect(enable)` | 启用/禁用自动重连（默认启用） |
| `setReconnectDelayCalcFunc(func)` | 设置自定义重连延迟计算函数 |
| `setCompressionPrefer(enable)` | 启用/禁用压缩偏好（必须在 initialize 之前调用） |
| `setFragmentSize(size)` | 设置发送分片大小（默认65535，0=不分片；必须在 initialize 之前调用） |

**State 状态枚举：**

| 状态 | 说明 |
|------|------|
| `kNone` | 未初始化 |
| `kInited` | 已初始化 |
| `kConnecting` | TCP 连接中 |
| `kHandshaking` | HTTP Upgrade 握手阶段 |
| `kConnected` | WebSocket 已连接 |

### WsFrame — WebSocket 帧

```cpp
struct WsFrame {
    enum class OpCode : uint8_t {
        kContinue = 0x0,   //! 继续
        kText     = 0x1,   //! 文本
        kBinary   = 0x2,   //! 二进制
        kClose    = 0x8,   //! 关闭连接
        kPing     = 0x9,   //! Ping
        kPong     = 0xA,   //! Pong
    };

    OpCode  opcode;         //! 帧操作码
    bool    fin = true;     //! 是否为最后一帧
    bool    rsv1 = false;   //! RSV1 位（压缩帧首帧为 true，RFC 7692）
    std::string payload;    //! 负载数据

    bool isControlFrame() const;   //! Close/Ping/Pong 为控制帧
    uint16_t closeCode() const;    //! 从 Close 帧中提取关闭码
    std::string closeReason() const; //! 从 Close 帧中提取关闭原因
};
```

### WsFrameParser — 增量帧解析器

适用于事件驱动场景的增量式 WebSocket 帧解析器，逐步从缓冲区中解析帧数据。

| 方法 | 说明 |
|------|------|
| `parse(data, size)` | 解析数据，返回已消费的字节数 |
| `state()` | 获取当前解析状态 |
| `getFrame()` | 获取解析完成的帧（仅 state == kFinished 时有效） |
| `reset()` | 重置解析器 |

### WsFrameBuilder — 帧构建器

用于构建 WebSocket 帧的静态辅助类。服务端帧不使用掩码，客户端帧必须使用掩码（RFC 6455）。

| 方法 | 说明 |
|------|------|
| `BuildTextFrame(text)` | 构建文本帧（服务端，不掩码） |
| `BuildBinaryFrame(data, len)` | 构建二进制帧（服务端，不掩码） |
| `BuildBinaryFrame(data)` | 构建二进制帧（服务端，vector 版本） |
| `BuildCloseFrame(code, reason)` | 构建关闭帧（服务端，不掩码） |
| `BuildPingFrame(data)` | 构建Ping帧（服务端，不掩码） |
| `BuildPongFrame(data)` | 构建Pong帧（服务端，不掩码） |
| `BuildFrame(opcode, fin, payload, len)` | 通用帧构建（服务端，不掩码） |
| `BuildMaskedTextFrame(text)` | 构建文本帧（客户端，掩码） |
| `BuildMaskedBinaryFrame(data, len)` | 构建二进制帧（客户端，掩码） |
| `BuildMaskedBinaryFrame(data)` | 构建二进制帧（客户端，掩码，vector版本） |
| `BuildMaskedCloseFrame(code, reason)` | 构建关闭帧（客户端，掩码） |
| `BuildMaskedPingFrame(data)` | 构建Ping帧（客户端，掩码） |
| `BuildMaskedPongFrame(data)` | 构建Pong帧（客户端，掩码） |
| `BuildMaskedFrame(opcode, fin, payload, len, mask_key)` | 通用帧构建（客户端，掩码） |

## 使用示例

### 服务端：群聊聊天室

> 完整示例见 `examples/websocket/chat_server/`

演示多个聊天室挂载在同一 HTTP 服务器上的不同 URL 路径。每个 ChatRoom 内含一个 WsServer 实例，管理 WebSocket 连接与聊天逻辑。客户端第一条文本消息作为用户名（登录），后续消息广播给所有已登录用户。

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
        ws_srv_.setTextMessageCallback([this](const WsServer::ConnToken &token, std::string &&text) {
            onTextMessage(token, std::move(text));
        });

        return true;
    }

    bool start() { return ws_srv_.start(); }
    void stop()  { ws_srv_.stop(); }
    void cleanup() { ws_srv_.cleanup(); }

  private:
    void onTextMessage(const WsServer::ConnToken &token, std::string &&text)
    {
        //! 第一条消息作为用户名
        auto it = conn_to_name_.find(token);
        if (it == conn_to_name_.end()) {
            conn_to_name_[token] = text;
            broadcast(text + " 上线");
        } else {
            broadcast(it->second + ": " + text);
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

    //! 创建 HTTP 服务器
    Server http_srv(sp_loop);
    http_srv.initialize(network::SockAddr::FromString("0.0.0.0:8080"), 1);

    //! 创建两个聊天室，挂载到不同 URL 路径
    ChatRoom chat_room_1(sp_loop, "聊天室1");
    ChatRoom chat_room_2(sp_loop, "聊天室2");

    chat_room_1.initialize(&http_srv, "/ws/chat-1");
    chat_room_2.initialize(&http_srv, "/ws/chat-2");

    //! 添加 HTTP 主页处理
    http_srv.use([&](ContextSptr ctx, const NextFunc &next) {
        if (ctx->req().url.path == "/") {
            ctx->res().status_code = StatusCode::k200_OK;
            ctx->res().body = "<h1>聊天服务器</h1>";
            return;
        }
        next();
    });

    //! 启动服务
    http_srv.start();
    chat_room_1.start();
    chat_room_2.start();

    //! ... 运行事件循环、处理 SIGINT、清理 ...
}
```

### 服务端：二进制 Echo

> 完整示例见 `examples/websocket/echo_bin/`

演示二进制 WebSocket 帧的处理。服务器将收到的二进制数据原样回传（echo），并每 5 秒通过 `send()` 的 `vector<uint8_t>` 版本向所有客户端推送统计帧（4字节头"STAT" + JSON字符串）。

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

        ws_srv_.setBinaryMessageCallback([this](const WsServer::ConnToken &token, std::vector<uint8_t> &&data) {
            onBinaryMessage(token, std::move(data));
        });
        ws_srv_.setTextMessageCallback([this](const WsServer::ConnToken &token, std::string &&text) {
            //! 此服务仅接收二进制帧
            ws_srv_.send(token, "此服务仅接收二进制帧");
        });

        //! 定时器：每 5 秒推送统计帧
        stat_timer_->initialize(std::chrono::milliseconds(5000), Event::Mode::kPersist);
        stat_timer_->setCallback([this] { onStatTimer(); });

        return true;
    }

  private:
    void onBinaryMessage(const WsServer::ConnToken &token, std::vector<uint8_t> &&data)
    {
        //! 二进制帧：原样回传
        ws_srv_.send(token, data);
    }

    void onStatTimer()
    {
        //! 构建二进制统计帧：4字节头"STAT" + JSON
        std::vector<uint8_t> stat_data;
        stat_data.insert(stat_data.end(), kStatHeader, kStatHeader + 4);
        stat_data.insert(stat_data.end(), json.begin(), json.end());

        for (const auto &token : conns_)
            ws_srv_.send(token, stat_data);
    }
};
```

### 客户端：聊天客户端

> 完整示例见 `examples/websocket/chat_client/`

演示 WebSocket 客户端连接到聊天服务器，从标准输入读取文本发送，并接收服务器推送的消息。

```cpp
#include <tbox/websocket/client/ws_client.h>

int main()
{
    auto sp_loop = Loop::New();

    WsClient ws_client(sp_loop);
    ws_client.initialize(SockAddr::FromString("127.0.0.1:8080"), "/ws/chat-1");

    ws_client.setConnectedCallback([&] {
        std::cout << "已连接！请输入用户名：" << std::endl;
        //! 启动标准输入读取
        sp_stdin_event->enable();
    });

    ws_client.setTextMessageCallback([&](std::string &&text) {
        std::cout << text << std::endl;
    });

    //! 设置二次退避重连策略
    ws_client.setReconnectDelayCalcFunc([](int fail_count) {
        return 1 << std::min(4, fail_count);
    });

    ws_client.start();

    //! ... 运行事件循环、处理 SIGINT、清理 ...
}
```

### 服务端：URL 路径匹配

```cpp
//! 前缀匹配：匹配 /ws/ 及其下的所有路径
ws_srv.initialize(&http_srv, "/ws/");

//! 全量匹配：仅匹配 /ws
ws_srv.initialize(&http_srv, "/ws");

//! 匹配所有：匹配所有 WebSocket 升级请求
ws_srv.initialize(&http_srv, "");
```

### 服务端：上下文数据

为客户端连接绑定自定义数据，实现每个连接的状态追踪：

```cpp
ws_srv.setConnectedCallback([](const WsServer::ConnToken &token) {
    //! 绑定用户会话对象
    auto session = new UserSession();
    ws_srv.setContext(token, session, [](void *p) { delete static_cast<UserSession*>(p); });
});

ws_srv.setTextMessageCallback([](const WsServer::ConnToken &token, std::string &&text) {
    //! 获取会话数据
    auto session = static_cast<UserSession*>(ws_srv.getContext(token));
    if (session != nullptr) {
        //! ... 使用会话数据处理消息 ...
    }
});
```

### 客户端：自定义重连延迟

```cpp
//! 指数退避：1秒, 2秒, 4秒, 8秒, ... 最大16秒
ws_client.setReconnectDelayCalcFunc([](int fail_count) {
    return 1 << std::min(4, fail_count);
});

//! 禁用自动重连
ws_client.setAutoReconnect(false);
```

## 压缩（RFC 7692 permessage-deflate）

websocket 模块支持 RFC 7692 定义的 `permessage-deflate` 压缩扩展。启用后，WebSocket 文本帧和二进制帧使用 DEFLATE (zlib) 压缩，显著减少带宽占用，尤其适用于重复性或大数据量的消息。

### 工作原理

1. **服务端**：在 `initialize()` 之前调用 `setCompressionEnable(true)`。若客户端在握手中请求了 `permessage-deflate`（通过 `Sec-WebSocket-Extensions: permessage-deflate` 头部），服务端在 101 响应中同意压缩。否则不使用压缩。

2. **客户端**：在 `initialize()` 之前调用 `setCompressionPrefer(true)`。客户端在握手中请求压缩扩展。若服务端同意，帧将压缩/解压；若服务端拒绝，通信继续不压缩。

3. **帧格式**：压缩数据帧的首帧设置 RSV1 位。控制帧（Close/Ping/Pong）永远不压缩。

4. **实现方式**：使用 raw DEFLATE，按 RFC 7692 Section 7.2.2 规则去除 4 字节尾部。每条消息独立压缩（no_context_takeover 模式），简化实现并确保兼容性。

### WsCompressionConfig — 压缩配置

```cpp
#include <tbox/websocket/ws_compressor.h>

struct WsCompressionConfig {
    bool enabled = false;                  //!< 是否启用压缩
    bool no_context_takeover = true;       //!< 不跨消息保留 zlib 上下文
    int  max_window_bits = 15;             //!< 最大窗口位数 (8~15)
};
```

### 服务端：启用压缩

```cpp
WsServer ws_srv(sp_loop);
ws_srv.setCompressionEnable(true);  //! 允许压缩（在 initialize 之前调用）
ws_srv.initialize(&http_srv, "/ws/chat");
```

### 客户端：偏好压缩

```cpp
WsClient ws_client(sp_loop);
ws_client.setCompressionPrefer(true);  //! 请求压缩（在 initialize 之前调用）
ws_client.initialize(SockAddr::FromString("127.0.0.1:8080"), "/ws/chat");
```

### 混合服务（部分路由压缩，部分不压缩）

```cpp
//! 聊天室启用压缩
WsServer ws_srv_compressed(sp_loop);
ws_srv_compressed.setCompressionEnable(true);
ws_srv_compressed.initialize(&http_srv, "/ws/chat");

//! Echo 服务不压缩
WsServer ws_srv_plain(sp_loop);
ws_srv_plain.initialize(&http_srv, "/ws/echo");
```

### 重要：压缩协商

- 压缩是**可选的**，在 HTTP Upgrade 握手阶段按连接协商。
- 若任一方不支持或拒绝压缩，帧将不压缩发送——对功能无任何影响。
- `WsFrame` 的 `rsv1` 字段标识接收到的帧是否被压缩。解压后 `rsv1` 被清除，用户回调中收到的 payload 是原始数据，透明无感。
- 压缩失败时优雅回退：若压缩或解压失败，系统回退到不压缩模式或报告错误。

## 常见场景

1. **实时推送**：将 WsServer 挂载到 HTTP 服务器上，向浏览器客户端推送实时数据
2. **聊天室**：同一 HTTP 服务器上不同 URL 路径承载多个聊天室
3. **二进制数据流**：回传二进制帧、发送带头部标识的结构化二进制数据
4. **IoT 设备通信**：客户端连接到服务器，发送状态更新并接收指令
5. **服务端心跳**：服务端发送 Ping 帧，客户端自动回复 Pong
6. **客户端自动重连**：断线后按指数退避策略自动重连
7. **HTTP + WebSocket 混合**：HTTP 提供 REST API 和静态页面；WebSocket 处理实时通信
8. **压缩通信**：启用 permessage-deflate 减少文本/二进制数据的带宽占用

## 注意事项

1. **WsServer 是 HTTP 中间件**：必须关联 `http::server::Server` 并在 HTTP 服务器启动前/后注册。WsServer 通过 `http_server->use()` 将自身注册为中间件。
2. **URL 路径匹配**：注意 `url_path` 是否以 `/` 结尾——决定前缀匹配还是全量匹配。空字符串匹配所有升级请求。
3. **ConnToken 操作**：所有客户端操作使用 `ConnToken`（cabinet::Token），而非指针。确保连接销毁后安全访问。
4. **Ping/Pong 自动回复**：WsServer 和 Client 在收到 Ping 帧时均自动回复 Pong 帧。
5. **Close 帧自动回复**：双方收到 Close 帧后自动回复 Close 帧，随后等待 TCP 断开。
6. **客户端帧掩码**：RFC 6455 规定客户端发送的所有帧必须掩码，服务端帧不掩码。
7. **客户端自动重连**：默认启用。断开后按配置的延迟策略自动重连。通过 `setReconnectDelayCalcFunc()` 自定义延迟。
8. **客户端握手**：Client 自动执行 HTTP Upgrade 握手，验证 101 响应、Sec-WebSocket-Accept、Upgrade/Connection 头部。握手失败时若自动重连已启用则自动重连。
9. **生命周期顺序**：WsServer 和 Client 均须遵循 initialize → start → stop → cleanup 顺序。
10. **线程安全**：所有回调在 Loop 线程中执行，跨线程操作须通过 `runInLoop()` 回到主线程。
11. **上下文数据**：WsServer 的 `setContext()/getContext()` 委托给底层 TcpConnection。在回调中可访问上下文数据，但连接断开后 `getContext()` 返回 `nullptr`。
12. **压缩**：在 WsServer 上调用 `setCompressionEnable(true)` 或在 WsClient 上调用 `setCompressionPrefer(true)` **必须在 `initialize()` 之前**。压缩按连接协商；若对方不支持，帧将不压缩发送，不影响功能。
13. **压缩回退**：若压缩/解压失败，系统打印警告并回退到不压缩发送。解压失败会触发错误回调。
14. **分片接收**：WsServer 和 WsClient 在内部缓存分片数据。只有接收完整消息（所有分片、fin=true）并解压后才回调业务层，回调中永远不会收到部分分片数据。
15. **分片发送**：当发送数据大于 `fragment_size` 时，自动分片发送。首帧携带原始 opcode 与 rsv1（如压缩），后续帧为 kContinue。调用 `setFragmentSize(size)` 配置分片大小（默认65535，设为0禁用分片），必须在 `initialize()` 之前。
16. **send(const char\*)**：WsServer 和 WsClient 都提供 `send(const char *str)` 重载，发送文本帧时不构造 std::string 中间对象，正确使用 kText opcode。

## 相关模块

- **http**：WsServer 运行在 http::server::Server 之上，作为 HTTP 中间件
- **event**：Server 和 Client 基于 Loop 运行
- **network**：通过 TcpConnector（客户端）和 TcpConnection 实现连接管理
- **crypto**：SHA1 计算 Sec-WebSocket-Accept
- **base**：提供 Cabinet 用于连接生命周期管理
