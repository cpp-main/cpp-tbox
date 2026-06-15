# JSON-RPC 模块 (jsonrpc)

## 是什么？

jsonrpc 模块提供了 JSON-RPC 2.0 协议的实现，支持请求/通知/回复的消息交互模式，可配合自定义传输层（如 TCP、WebSocket）使用。

## 为什么需要它？

在需要远程过程调用（RPC）的场景中，JSON-RPC 是一种轻量级且易于实现的协议。jsonrpc 模块封装了消息编解码、请求超时管理、异步回复等机制，让开发者只需关注服务方法的实现。

## 头文件

```cpp
#include <tbox/jsonrpc/rpc.h>     //! RPC 核心类
#include <tbox/jsonrpc/proto.h>   //! 协议抽象基类
#include <tbox/jsonrpc/types.h>   //! 类型定义
```

## 核心类与接口

### Rpc — RPC 核心

| 方法 | 说明 |
|------|------|
| `Rpc(loop, id_type)` | 构造，指定 ID 类型（kInt 或 kString） |
| `initialize(proto, timeout_sec)` | 初始化协议和超时时间 |
| `addService(method, cb)` | 注册方法服务 |
| `removeService(method)` | 删除方法服务 |
| `request(method, params, cb)` | 发送请求（需回复） |
| `request(method, cb)` | 发送请求（无参数） |
| `notify(method, params)` | 发送通知（不需要回复） |
| `notify(method)` | 发送通知（无参数） |
| `respondResult(int_id, result)` | 异步回复成功结果 |
| `respondError(int_id, errcode, message)` | 异步回复错误 |
| `clear()` | 清除缓存数据 |

### ServiceCallback — 方法回调

```cpp
using ServiceCallback = std::function<bool(int int_id, const Json &params, Response &response)>;
```

- 返回 `true`：同步回复，函数返回后自动根据 response 进行回复
- 返回 `false`：异步回复，后续通过 `respondResult/respondError` 手动回复

### Proto — 协议传输层

Proto 是协议的传输层抽象，需要用户实现具体的传输方式（如基于 TcpConnection）：

| 方法 | 说明 |
|------|------|
| `setRecvCallback(req_cb, rsp_cb)` | 设置接收回调 |
| `setSendCallback(send_cb)` | 设置发送回调 |
| `onRecvData(data, size)` | 处理收到的数据（需子类实现） |

## 使用示例

### 请求端 (Ping)

> 完整示例见 `examples/jsonrpc/req_rsp/ping/`

```cpp
Rpc rpc(sp_loop, IdType::kInt);
rpc.initialize(proto, 30);  //! 超时30秒

//! 发送请求，等待回复
rpc.request("ping", Json::object{{"data", "hello"}},
    [](const Response &rsp) {
        if (rsp.errcode == 0)
            LogInfo("result: %s", rsp.result.dump().c_str());
        else
            LogErr("error: %d, %s", rsp.errcode, rsp.message.c_str());
    }
);
```

### 服务端 (Pong)

> 完整示例见 `examples/jsonrpc/req_rsp/pong/`

```cpp
Rpc rpc(sp_loop, IdType::kInt);
rpc.initialize(proto, 30);

//! 注册服务方法
rpc.addService("ping",
    [](int int_id, const Json &params, Response &response) {
        //! 同步回复
        response.errcode = 0;
        response.result = Json::object{{"echo", params["data"]}};
        return true;
    }
);
```

### 异步回复

```cpp
rpc.addService("async_query",
    [](int int_id, const Json &params, Response &response) {
        //! 异步处理：不立即回复，稍后通过 respondResult 回复
        //! 返回 false 表示不自动回复
        thread_pool.execute(
            [int_id, params] { /* 后台查询 */ },
            [int_id, &rpc] {
                rpc.respondResult(int_id, Json::object{{"status", "ok"}});
            }
        );
        return false;
    }
);
```

### 通知（不需要回复）

```cpp
//! 发送通知
rpc.notify("event", Json::object{{"type", "alert"}});
```

### 消息通信 (Ping/Pong 单向)

> 完整示例见 `examples/jsonrpc/message/ping/` 和 `pong/`

适用于简单的消息传递场景，无需请求-回复模式。

## 常见场景

1. **请求-回复**：客户端发送请求，服务端回复结果
2. **异步处理**：服务端收到请求后异步处理，稍后回复
3. **事件通知**：一方发送通知消息，另一方仅接收不回复
4. **超时管理**：请求超时自动触发超时回调

## 注意事项

1. **ID 类型**：Int ID 自增分配简单高效；String ID（如 UUID）更安全但开销更大
2. **Proto 实现**：需要自行实现 Proto 的 onRecvData() 方法，解析传输层数据
3. **超时时间**：request 发出的请求如果在超时时间内没有收到回复，将触发超时回调（errcode != 0）
4. **异步回复的 int_id**：异步回复时需要保存 int_id，后续用它回复
5. **clear() 清除状态**：重置 RPC 对象的缓存数据，恢复到未收发数据的状态

## 相关模块

- **event**：基于 Loop 运行
- **eventx**：使用 TimeoutMonitor 实现请求超时管理
- **network**：可基于 TcpConnection 实现 Proto 传输层
- **base**：提供 Json、Cabinet/Token 等基础设施
