# HTTP 服务模块 (http)

## 是什么？

http 模块提供了轻量级 HTTP 服务器和客户端实现，设计参考了 Node.js Express 的中间件模式，接口简洁，使用方便。它旨在补全服务程序对外提供 RESTful API 的能力，而非取代 Apache/Nginx 等成熟 HTTP 服务器。

在客户端侧，提供了异步 HTTP 客户端，支持自动重连、请求超时和便捷的请求方法，方便 C++ 程序向其它服务发起 HTTP 请求。

## 为什么需要它？

在嵌入式设备或小型服务程序中，需要提供简单的 HTTP API 或 Web 页面，但不想引入重量级 HTTP 服务器。http 模块让 C++ 程序能直接提供 HTTP 服务，支持中间件链式处理、路由分发、文件下载、表单上传等功能。

在客户端侧，可能需要向其它服务发起 HTTP 请求（如调用 REST API、上传数据等）。http Client 类提供了异步 HTTP 客户端，支持自动重连、请求超时和便捷的请求方法。

## 头文件

```cpp
#include <tbox/http/server/server.h>                         //! HTTP 服务端
#include <tbox/http/server/context.h>                        //! 请求上下文
#include <tbox/http/server/middleware.h>                     //! 中间件基类
#include <tbox/http/server/middlewares/router_middleware.h>  //! 路由中间件
#include <tbox/http/server/middlewares/file_downloader_middleware.h> //! 文件下载中间件
#include <tbox/http/server/middlewares/form_data_middleware.h>       //! 表单数据中间件
#include <tbox/http/client/client.h>                         //! HTTP 客户端
#include <tbox/http/request.h>                               //! HTTP 请求
#include <tbox/http/respond.h>                               //! HTTP 响应
#include <tbox/http/common.h>                                //! HTTP 公共定义
#include <tbox/http/url.h>                                   //! URL 解析
```

## 核心类与接口

### Server — HTTP 服务端

| 方法 | 说明 |
|------|------|
| `Server(loop)` | 构造 |
| `initialize(bind_addr, backlog)` | 初始化绑定地址 |
| `use(handler)` | 添加请求处理函数 |
| `use(middleware)` | 添加中间件 |
| `start()` | 启动服务 |
| `stop()` | 停止服务 |
| `cleanup()` | 清理 |
| `setContextLogEnable(enable)` | 启用/禁用详细收发日志（调试用） |

### Client — HTTP 客户端

| 方法 | 说明 |
|------|------|
| `Client(loop)` | 构造 |
| `initialize(server_addr)` | 初始化，设置目标服务器地址 (SockAddr) |
| `start()` | 开始连接服务器 |
| `stop()` | 停止/断开连接 |
| `cleanup()` | 清理（与 initialize 逆操作） |
| `state()` | 获取当前状态 (None/Inited/Connecting/Connected/ReconnWaiting) |
| `request(req, cb)` | 发送完整 Request 对象，指定回复回调 |
| `request(method, path, cb)` | 便捷方法：指定 Method 和 path |
| `request(method, path, body, headers, cb)` | 便捷方法：指定 Method、path、body、headers |
| `setAutoReconnect(enable)` | 启用/禁用自动重连 |
| `setReconnectDelayCalcFunc(func)` | 设置自定义重连延迟计算函数 |
| `setRequestTimeout(ms)` | 设置请求超时时间（默认 30 秒） |
| `setContextLogEnable(enable)` | 启用/禁用详细收发日志 |
| `setConnectedCallback(cb)` | 设置连接成功回调 |
| `setConnectFailCallback(cb)` | 设置连接失败回调 |
| `setDisconnectedCallback(cb)` | 设置断线回调 |

**State 状态枚举：**

| 状态 | 说明 |
|------|------|
| `kNone` | 未初始化 |
| `kInited` | 已初始化 |
| `kConnecting` | 连接中 |
| `kConnected` | 已连接 |
| `kReconnWaiting` | 断连等待重连中 |

### Context — 请求上下文

| 方法 | 说明 |
|------|------|
| `ctx.req()` | 获取 Request 对象 |
| `ctx.res()` | 获取 Respond 对象（done 后不可再使用） |

### Request / Respond 结构

```cpp
struct Request {
    Method  method;      //! GET/POST/PUT/DELETE 等
    HttpVer http_ver;    //! HTTP 版本
    Url::Path url;       //! 请求路径
    Headers headers;     //! 请求头
    std::string body;    //! 请求体
};

struct Respond {
    HttpVer http_ver;
    StatusCode status_code;  //! 200/404/500 等
    Headers headers;
    std::string body;
};
```

### Middleware — 中间件基类

中间件是 Express 模式的核心。每个中间件接收 Context 和 NextFunc，可以选择处理请求或调用 next() 传递给下一个中间件。

```cpp
class Middleware {
    virtual void handle(ContextSptr ctx, const NextFunc &next) = 0;
};
```

### RouterMiddleware — 路由中间件

提供类似 Express Router 的路由分发：

```cpp
RouterMiddleware router;
router.get("/", handler);      //! GET 请求
router.post("/api", handler);  //! POST 请求
router.put("/data", handler);  //! PUT 请求
router.del("/item", handler);  //! DELETE 请求
```

### FileDownloaderMiddleware — 文件下载中间件

支持文件下载、Range 请求、ETag 缓存和 CORS，适配 iOS AVPlayer 视频播放。

## 使用示例

### Server：最简单的 HTTP 服务

> 完整示例见 `examples/http/server/simple/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/http/server/server.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::http;
using namespace tbox::http::server;

int main() {
    LogOutput_Enable();

    auto sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    Server srv(sp_loop);
    srv.initialize(network::SockAddr::FromString("0.0.0.0:12345"), 1);
    srv.start();

    //! 添加请求处理
    srv.use(
        [&](ContextSptr ctx, const NextFunc &next) {
            ctx->res().status_code = StatusCode::k200_OK;
            ctx->res().body = "Hello!";
        }
    );

    //! 监听退出信号
    auto sp_sig = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_sig] { delete sp_sig; });
    sp_sig->initialize(SIGINT, Event::Mode::kPersist);
    sp_sig->enable();
    sp_sig->setCallback([&] (int) { srv.stop(); sp_loop->exitLoop(); });

    sp_loop->runLoop();
    srv.cleanup();

    LogOutput_Disable();
    return 0;
}
```

### Server：路由分发

> 完整示例见 `examples/http/server/router/`

```cpp
RouterMiddleware router;
srv.use(&router);

router
    .get("/", [](ContextSptr ctx, const NextFunc &next) {
        ctx->res().status_code = StatusCode::k200_OK;
        ctx->res().headers["Content-Type"] = "text/html; charset=UTF-8";
        ctx->res().body = "<h1>Home</h1>";
    })
    .get("/api/data", [](ContextSptr ctx, const NextFunc &next) {
        ctx->res().status_code = StatusCode::k200_OK;
        ctx->res().headers["Content-Type"] = "application/json";
        ctx->res().body = "{\"status\":\"ok\"}";
    })
    .post("/api/upload", [](ContextSptr ctx, const NextFunc &next) {
        //! 处理 POST 上传
        ctx->res().status_code = StatusCode::k200_OK;
    });
```

### Server：异步响应

> 完整示例见 `examples/http/server/async_respond/`

```cpp
srv.use(
    [&](ContextSptr ctx, const NextFunc &next) {
        //! 不立即回复，稍后异步处理
        ctx->res().status_code = StatusCode::k200_OK;

        //! 在其它线程完成后回复
        tp.execute(
            [] { /* 后台耗时操作 */ },
            [ctx] { ctx->res().body = "async result"; /* 回复 */ }
        );
    }
);
```

### Server：文件下载

> 完整示例见 `examples/http/server/file_download/`

```cpp
FileDownloaderMiddleware file_dl;
file_dl.setRootPath("/data/files");  //! 设置文件根目录
srv.use(&file_dl);
```

### Server：表单上传

> 完整示例见 `examples/http/server/form_data/`

```cpp
FormDataMiddleware form_data;
srv.use(&form_data);

router.post("/upload", [](ContextSptr ctx, const NextFunc &next) {
    //! 获取上传的文件数据
    auto files = ctx->req().headers;  //! 通过 FormDataMiddleware 处理后的数据
});
```

### Client：简单的 HTTP 客户端

> 完整示例见 `examples/http/client/simple/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/http/client/client.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

using namespace tbox;
using namespace tbox::event;
using namespace tbox::http;
using namespace tbox::http::client;

int main() {
    LogOutput_Enable();

    auto sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    Client http_client(sp_loop);
    http_client.initialize(network::SockAddr::FromString("127.0.0.1:12345"));
    http_client.setAutoReconnect(true);
    http_client.setRequestTimeout(std::chrono::seconds(10));
    http_client.start();

    //! 简单 GET 请求
    http_client.request(Method::kGet, "/",
        [](const Respond &res) {
            LogInfo("GET / => status: %d, body: %s",
                    (int)res.status_code, res.body.c_str());
        });

    //! POST 请求（带 body 和 headers）
    http_client.request(Method::kPost, "/api/data",
        "{\"key\":\"value\"}",
        {{"Content-Type", "application/json"}},
        [](const Respond &res) {
            LogInfo("POST /api/data => status: %d", (int)res.status_code);
        });

    //! 完整 Request 对象
    Request req;
    req.method = Method::kPut;
    req.http_ver = HttpVer::k1_1;
    req.url.path = "/api/update";
    req.headers["Content-Type"] = "application/json";
    req.body = "{\"id\":123}";
    http_client.request(req,
        [](const Respond &res) {
            LogInfo("PUT /api/update => status: %d", (int)res.status_code);
        });

    //! 监听退出信号
    auto sp_sig = sp_loop->newSignalEvent();
    SetScopeExitAction([sp_sig] { delete sp_sig; });
    sp_sig->initialize(SIGINT, Event::Mode::kPersist);
    sp_sig->enable();
    sp_sig->setCallback([&] (int) { http_client.stop(); sp_loop->exitLoop(); });

    sp_loop->runLoop();
    http_client.cleanup();

    LogOutput_Disable();
    return 0;
}
```

### Client：连接回调

```cpp
http_client.setConnectedCallback(
    [] { LogInfo("连接成功"); }
);
http_client.setConnectFailCallback(
    [] { LogWarn("连接失败"); }
);
http_client.setDisconnectedCallback(
    [] { LogInfo("连接断开"); }
);
```

### Client：自定义重连延迟

```cpp
//! 指数退避：1秒, 2秒, 4秒, 8秒, ... 最大30秒
http_client.setReconnectDelayCalcFunc(
    [](int fail_count) {
        int delay = 1 << fail_count;
        return delay > 30 ? 30 : delay;
    }
);
```

## 常见场景

1. **RESTful API**：使用 RouterMiddleware 分发 GET/POST/PUT/DELETE 请求
2. **静态文件服务**：使用 FileDownloaderMiddleware 提供文件下载
3. **异步处理**：收到请求后不立即回复，在后台线程处理完成后异步响应
4. **中间件链**：多个中间件依次处理请求（如日志→认证→业务）
5. **视频流**：FileDownloaderMiddleware 支持 Range 请求，适配 iOS AVPlayer
6. **调用外部 API**：使用 Client 向其它服务发起 HTTP 请求
7. **服务间通信**：使用 Client 配合自动重连，实现可靠的服务间 HTTP 调用

## SSE — Server-Sent Events（服务端推送事件）

http 模块还包含 SSE（Server-Sent Events）子包，实现基于 W3C/WHATWG EventSource 规范的服务端事件推送。SSE 使用标准 HTTP 长响应（200 OK）向浏览器流式推送事件，不像 WebSocket 需要协议升级。

### 头文件

```cpp
#include <tbox/http/server/sse/sse_event.h>    //! SSE 事件数据结构
#include <tbox/http/server/sse/sse_server.h>    //! SSE 服务端
#include <tbox/http/server/sse/sse_connection.h> //! SSE 连接（内部类）
```

### SseServer — SSE 服务端

SseServer 运行在 HTTP 服务器之上，作为中间件存在。它检测 SSE 请求（Accept 头包含 text/event-stream），设置 200 OK 响应头，通过 `upgrade_cb` 机制接管 TcpConnection，提供持续的事件推送。

| 方法 | 说明 |
|------|------|
| `SseServer(loop)` | 构造 |
| `initialize(http_server, url_path)` | 初始化：关联到 HTTP 服务器；`url_path` 控制 URL 匹配规则 |
| `start()` | 启动（注册为 HTTP 中间件） |
| `stop()` | 停止（反注册中间件，关闭所有 SSE 连接） |
| `cleanup()` | 清理 |
| `state()` | 获取当前状态 (None/Inited/Running) |
| `send(client, data)` | 向指定客户端发送数据（简单文本） |
| `send(client, event)` | 向指定客户端发送 SseEvent |
| `sendToAll(data)` | 向所有客户端广播数据 |
| `sendToAll(event)` | 向所有客户端广播 SseEvent |
| `close(client)` | 关闭指定客户端连接 |
| `sendHeartbeat(client, comment)` | 发送心跳注释行 |
| `setHeartbeatInterval(ms)` | 设置自动心跳间隔（默认 0 = 禁用） |
| `isClientValid(client)` | 检查客户端连接是否有效 |
| `peerAddr(client)` | 获取客户端地址 |
| `getLastEventId(client)` | 获取浏览器重连时的 Last-Event-ID |
| `getUrl(client)` | 获取客户端连接的 URL 路径 |
| `setContext(client, ctx, deleter)` | 设置上下文数据 |
| `getContext(client)` | 获取上下文数据 |
| `setConnectedCallback(cb)` | 设置回调：客户端连接 |
| `setDisconnectedCallback(cb)` | 设置回调：客户端断开 |
| `IsSseRequest(req)` | 静态方法：检查是否为 SSE 请求 |

**URL 路径匹配规则：** 与 WsServer 一致——url_path 以 `/` 结尾为前缀匹配，不以 `/` 结尾为全量匹配，空字符串匹配所有。

**SSE 与 WebSocket 对比：**

| 特性 | WebSocket | SSE |
|------|-----------|------|
| HTTP 状态码 | 101 Switching Protocols | 200 OK |
| 数据方向 | 双向 | 仅服务端→客户端 |
| 数据格式 | 二进制帧 | 纯文本（data:/event:/id: 字段） |
| 客户端消息回调 | 有 | 无（单向） |
| 心跳 | Ping/Pong 帧 | 注释行 + 定时器 |
| 重连机制 | 自行实现 | 浏览器自动重连 + Last-Event-ID |
| 模块位置 | 独立 `websocket` 模块 | `http` 模块内 |

### SseEvent — SSE 事件数据结构

```cpp
struct SseEvent {
    std::string id;      //! 事件ID（可选），用于 Last-Event-ID 断线续传
    std::string event;   //! 事件类型（可选，默认 "message"）
    std::string data;    //! 数据（必须，支持多行）
    int retry = 0;       //! 重连间隔毫秒数（可选）

    //! 将事件格式化为 SSE 文本协议格式
    //! 多行 data 自动拆分为多个 `data:` 行
    std::string toString() const;
};
```

### SSE 示例：事件推送

> 完整示例见 `examples/http/server/sse/`

```cpp
#include <tbox/http/server/server.h>
#include <tbox/http/server/sse/sse_server.h>

SseServer sse_srv(sp_loop);
sse_srv.initialize(&http_srv, "/sse/events");
sse_srv.setHeartbeatInterval(std::chrono::seconds(15));

sse_srv.setConnectedCallback([](const SseServer::ConnToken &token) {
    LogInfo("sse 客户端已连接");
    sse_srv.send(token, "欢迎！");
});

//! 每 5 秒推送事件
SseEvent evt;
evt.id = "42";
evt.event = "tick";
evt.data = "{\"time\":\"2026-06-16 10:30:00\"}";
sse_srv.sendToAll(evt);
```

## 注意事项

1. **中间件调用顺序**：`use()` 添加的中间件按添加顺序执行
2. **NextFunc**：中间件中调用 `next()` 才会将请求传递给下一个中间件；不调用 next 则终止链
3. **Context 的 res()**：调用 `done()` 后不可再使用 `res()`
4. **文件下载安全性**：FileDownloaderMiddleware 需正确设置根目录，防止路径遍历攻击
5. **线程安全**：HTTP 请求处理在 Loop 线程中执行，异步操作需通过 runInLoop 回到主线程
6. **Client 生命周期**：必须遵循 initialize → start → stop → cleanup 顺序；在 start() 之前调用 request() 会将请求缓存，连接建立后自动发送
7. **Client 请求超时**：每个请求有独立的超时定时器，超时后回调返回 StatusCode::k408_RequestTimeout
8. **Client 断线处理**：断线时所有 pending 请求会收到 StatusCode::k504_GatewayTimeout 的错误回调；重连后需重新发起请求
9. **Client 请求顺序**：响应按 FIFO 顺序与请求匹配，队列设计兼容管线化和非管线化的 HTTP/1.1

## 相关模块

- **event**：Server 和 Client 基于 Loop 运行
- **network**：基于 TcpServer（服务端）/ TcpClient（客户端）实现 HTTP 连接管理
- **eventx**：异步响应需要 ThreadPool
- **base**：提供 StatusCode、Method 等定义
