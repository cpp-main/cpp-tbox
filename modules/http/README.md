# HTTP 模块

本模块的设计初宗并非为了取代 Apache, Nginx 这类已非常成熟的 HTTP 服务器，而是为了补全
我们服务程序中无法对外提供RESTfull API的缺口。

本模块在设计时参考了 node.js 中 Express 的中间件设计思想。接口简洁，使用方便。

## Server 端

示例：
```c++

using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;
using namespace tbox::http;
using namespace tbox::http::server;

//! 假设已存在Loop的实例指针 sp_loop

Server srv(sp_loop);
if (!srv.initialize(network::SockAddr::FromString(bind_addr), 1)) {
    LogErr("init srv fail");
    return 0;
}

srv.start();

//! 添加请求处理
srv.use(
    [&](ContextSptr ctx, const NextFunc &next) {
        ctx->res().status_code = StatusCode::k200_OK;
        ctx->res().body = "Hello!";
    }
);

//! 其它杂项，比如：设置退出中断信号

sp_loop->runLoop();
srv.cleanup();

```

具体使用，请参考 `examples/http/server/` 下的示例。

## Client 端

Client 类实现了异步 HTTP 客户端，基于 TcpClient 实现连接管理与自动重连。

### 生命周期

与其它 tbox 组件一致，遵循 `initialize → start → stop → cleanup` 的生命周期模式。

### 示例

```c++

using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;
using namespace tbox::http;
using namespace tbox::http::client;

//! 假设已存在Loop的实例指针 sp_loop

Client http_client(sp_loop);
if (!http_client.initialize(network::SockAddr::FromString("127.0.0.1:12345"))) {
    LogErr("init http_client fail");
    return 0;
}

http_client.setAutoReconnect(true);
http_client.setRequestTimeout(std::chrono::seconds(10));
http_client.start();

//! 简单 GET 请求
http_client.request(Method::kGet, "/",
    [](const Respond &res) {
        LogInfo("GET / => status: %d, body: %s",
                (int)res.status_code, res.body.c_str());
    });

//! POST 请求
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

sp_loop->runLoop();
http_client.cleanup();

```

具体使用，请参考 `examples/http/client/` 下的示例。

### 特性说明

| 特性 | 说明 |
|------|------|
| 自动重连 | 断线后自动重连服务器，可通过 setAutoReconnect() 控制 |
| 请求超时 | 每个请求有独立的超时定时器，默认 30 秒 |
| 请求缓存 | 未连接时请求会被缓存，连接建立后自动发送 |
| 回调通知 | 支持连接成功、连接失败、断线等回调通知 |
| 便捷方法 | request() 支持 3 种重载，降低使用门槛 |
