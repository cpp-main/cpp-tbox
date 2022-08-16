# HTTP 模块

本模块的设计初宗并非为了取代 Apache, Nginx 这类已非常成熟的 HTTP 服务器，而是为了补全
我们服务程序中无法对外提供RESTfull API的缺口。

本模块在设计时参考了 node.js 中 Express 的中间件设计思想。接口简洁，使用方便。

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

具体使用，请参考 example/ 下的示例。
