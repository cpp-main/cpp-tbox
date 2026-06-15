# HTTP Service Module (http)

## What is it?

The http module provides lightweight HTTP server and client implementations, designed with reference to the Node.js Express middleware pattern. It features a concise interface and ease of use. It is intended to supplement service programs with the ability to expose RESTful APIs, rather than to replace mature HTTP servers like Apache/Nginx.

## Why do you need it?

In embedded devices or small service programs, you may need to provide simple HTTP APIs or web pages without introducing a heavyweight HTTP server. The http module allows C++ programs to serve HTTP directly, supporting middleware chain processing, route dispatching, file downloading, form uploading, and more.

On the client side, you may need to make HTTP requests to other services (e.g., calling REST APIs, uploading data). The http Client class provides an asynchronous HTTP client with auto-reconnect, request timeout, and convenient request methods.

## Header Files

```cpp
#include <tbox/http/server/server.h>                         //! HTTP server
#include <tbox/http/server/context.h>                        //! Request context
#include <tbox/http/server/middleware.h>                     //! Middleware base class
#include <tbox/http/server/middlewares/router_middleware.h>  //! Router middleware
#include <tbox/http/server/middlewares/file_downloader_middleware.h> //! File downloader middleware
#include <tbox/http/server/middlewares/form_data_middleware.h>       //! Form data middleware
#include <tbox/http/client/client.h>                         //! HTTP client
#include <tbox/http/request.h>                               //! HTTP request
#include <tbox/http/respond.h>                               //! HTTP response
#include <tbox/http/common.h>                                //! HTTP common definitions
#include <tbox/http/url.h>                                   //! URL parsing
```

## Core Classes and Interfaces

### Server — HTTP Server

| Method | Description |
|------|------|
| `Server(loop)` | Constructor |
| `initialize(bind_addr, backlog)` | Initialize with bind address |
| `use(handler)` | Add a request handler function |
| `use(middleware)` | Add a middleware |
| `start()` | Start the server |
| `stop()` | Stop the server |
| `cleanup()` | Cleanup |
| `setContextLogEnable(enable)` | Enable/disable detailed send/receive logs (for debugging) |

### Client — HTTP Client

| Method | Description |
|------|------|
| `Client(loop)` | Constructor |
| `initialize(server_addr)` | Initialize with server address (SockAddr) |
| `start()` | Start connecting to the server |
| `stop()` | Stop/disconnect from the server |
| `cleanup()` | Cleanup (inverse of initialize) |
| `state()` | Get current state (None/Inited/Connecting/Connected/ReconnWaiting) |
| `request(req, cb)` | Send a full Request object with response callback |
| `request(method, path, cb)` | Convenience: send request with Method and path |
| `request(method, path, body, headers, cb)` | Convenience: send request with body and headers |
| `setAutoReconnect(enable)` | Enable/disable auto-reconnect |
| `setReconnectDelayCalcFunc(func)` | Set custom reconnect delay calculation |
| `setRequestTimeout(ms)` | Set request timeout (default: 30 seconds) |
| `setContextLogEnable(enable)` | Enable/disable detailed send/receive logs |
| `setConnectedCallback(cb)` | Set callback for successful connection |
| `setConnectFailCallback(cb)` | Set callback for connection failure |
| `setDisconnectedCallback(cb)` | Set callback for disconnection |

**State enum:**

| State | Description |
|-------|-------------|
| `kNone` | Not initialized |
| `kInited` | Initialized |
| `kConnecting` | Connecting to server |
| `kConnected` | Connected to server |
| `kReconnWaiting` | Disconnected, waiting for auto-reconnect |

### Context — Request Context

| Method | Description |
|------|------|
| `ctx.req()` | Get the Request object |
| `ctx.res()` | Get the Respond object (cannot be used after done) |

### Request / Respond Structures

```cpp
struct Request {
    Method  method;      //! GET/POST/PUT/DELETE etc.
    HttpVer http_ver;    //! HTTP version
    Url::Path url;       //! Request path
    Headers headers;     //! Request headers
    std::string body;    //! Request body
};

struct Respond {
    HttpVer http_ver;
    StatusCode status_code;  //! 200/404/500 etc.
    Headers headers;
    std::string body;
};
```

### Middleware — Middleware Base Class

Middleware is the core of the Express pattern. Each middleware receives a Context and a NextFunc, and can either handle the request or call next() to pass it to the next middleware.

```cpp
class Middleware {
    virtual void handle(ContextSptr ctx, const NextFunc &next) = 0;
};
```

### RouterMiddleware — Router Middleware

Provides route dispatching similar to Express Router:

```cpp
RouterMiddleware router;
router.get("/", handler);      //! GET request
router.post("/api", handler);  //! POST request
router.put("/data", handler);  //! PUT request
router.del("/item", handler);  //! DELETE request
```

### FileDownloaderMiddleware — File Downloader Middleware

Supports file downloading, Range requests, ETag caching, and CORS, compatible with iOS AVPlayer video playback.

## Usage Examples

### Server: Simplest HTTP Service

> Full example in `examples/http/server/simple/`

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

    //! Add request handler
    srv.use(
        [&](ContextSptr ctx, const NextFunc &next) {
            ctx->res().status_code = StatusCode::k200_OK;
            ctx->res().body = "Hello!";
        }
    );

    //! Listen for exit signal
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

### Server: Route Dispatching

> Full example in `examples/http/server/router/`

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
        //! Handle POST upload
        ctx->res().status_code = StatusCode::k200_OK;
    });
```

### Server: Asynchronous Response

> Full example in `examples/http/server/async_respond/`

```cpp
srv.use(
    [&](ContextSptr ctx, const NextFunc &next) {
        //! Do not reply immediately, process asynchronously later
        ctx->res().status_code = StatusCode::k200_OK;

        //! Reply after completion in another thread
        tp.execute(
            [] { /* Background time-consuming operation */ },
            [ctx] { ctx->res().body = "async result"; /* Reply */ }
        );
    }
);
```

### Server: File Downloading

> Full example in `examples/http/server/file_download/`

```cpp
FileDownloaderMiddleware file_dl;
file_dl.setRootPath("/data/files");  //! Set file root directory
srv.use(&file_dl);
```

### Server: Form Upload

> Full example in `examples/http/server/form_data/`

```cpp
FormDataMiddleware form_data;
srv.use(&form_data);

router.post("/upload", [](ContextSptr ctx, const NextFunc &next) {
    //! Get uploaded file data
    auto files = ctx->req().headers;  //! Data processed by FormDataMiddleware
});
```

### Client: Simple HTTP Client

> Full example in `examples/http/client/simple/`

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

    //! Simple GET request
    http_client.request(Method::kGet, "/",
        [](const Respond &res) {
            LogInfo("GET / => status: %d, body: %s",
                    (int)res.status_code, res.body.c_str());
        });

    //! POST request with body and headers
    http_client.request(Method::kPost, "/api/data",
        "{\"key\":\"value\"}",
        {{"Content-Type", "application/json"}},
        [](const Respond &res) {
            LogInfo("POST /api/data => status: %d", (int)res.status_code);
        });

    //! Full Request object
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

    //! Listen for exit signal
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

### Client: Connection Callbacks

```cpp
http_client.setConnectedCallback(
    [] { LogInfo("connected to server"); }
);
http_client.setConnectFailCallback(
    [] { LogWarn("connect failed"); }
);
http_client.setDisconnectedCallback(
    [] { LogInfo("disconnected from server"); }
);
```

### Client: Custom Reconnect Delay

```cpp
//! Exponential backoff: 1s, 2s, 4s, 8s, ... max 30s
http_client.setReconnectDelayCalcFunc(
    [](int fail_count) {
        int delay = 1 << fail_count;
        return delay > 30 ? 30 : delay;
    }
);
```

## Common Scenarios

1. **RESTful API**: Use RouterMiddleware to dispatch GET/POST/PUT/DELETE requests
2. **Static file serving**: Use FileDownloaderMiddleware to provide file downloads
3. **Asynchronous processing**: Receive a request without replying immediately, then respond asynchronously after background thread processing completes
4. **Middleware chain**: Multiple middleware process requests in sequence (e.g., logging -> authentication -> business logic)
5. **Video streaming**: FileDownloaderMiddleware supports Range requests, compatible with iOS AVPlayer
6. **Calling external APIs**: Use Client to make HTTP requests to other services
7. **Service-to-service communication**: Client with auto-reconnect for reliable inter-service HTTP calls

## Important Notes

1. **Middleware invocation order**: Middleware added via `use()` executes in the order it was added
2. **NextFunc**: Calling `next()` in a middleware passes the request to the next middleware; not calling next terminates the chain
3. **Context's res()**: You cannot use `res()` after calling `done()`
4. **File download security**: FileDownloaderMiddleware must be configured with the correct root directory to prevent path traversal attacks
5. **Thread safety**: HTTP request handling runs in the Loop thread; asynchronous operations must use runInLoop to return to the main thread
6. **Client lifecycle**: Must follow initialize -> start -> stop -> cleanup; calling request() before start() will cache the request until connected
7. **Client timeout**: Each request has an independent timeout timer; timeout triggers a callback with StatusCode::k408_RequestTimeout
8. **Client disconnection**: When disconnected, all pending requests receive error callbacks with StatusCode::k504_GatewayTimeout; cached requests are sent upon reconnection
9. **Client request order**: Responses are matched with requests in FIFO order; the request queue design is compatible with both pipelined and non-pipelined HTTP/1.1

## Related Modules

- **event**: Server and Client run based on Loop
- **network**: HTTP connection management implemented via TcpServer (server) / TcpClient (client)
- **eventx**: Asynchronous responses require ThreadPool
- **base**: Provides StatusCode, Method, and other definitions
