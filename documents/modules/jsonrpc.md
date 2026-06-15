# JSON-RPC Module (jsonrpc)

## What is it?

The jsonrpc module provides an implementation of the JSON-RPC 2.0 protocol, supporting request/notification/response message interaction patterns, and can be used with custom transport layers (such as TCP, WebSocket).

## Why do you need it?

In scenarios requiring Remote Procedure Calls (RPC), JSON-RPC is a lightweight and easy-to-implement protocol. The jsonrpc module encapsulates message encoding/decoding, request timeout management, asynchronous response, and other mechanisms, allowing developers to focus only on service method implementation.

## Header Files

```cpp
#include <tbox/jsonrpc/rpc.h>     //! RPC core class
#include <tbox/jsonrpc/proto.h>   //! Protocol abstract base class
#include <tbox/jsonrpc/types.h>   //! Type definitions
```

## Core Classes and Interfaces

### Rpc — RPC Core

| Method | Description |
|--------|-------------|
| `Rpc(loop, id_type)` | Construct, specifying ID type (kInt or kString) |
| `initialize(proto, timeout_sec)` | Initialize protocol and timeout |
| `addService(method, cb)` | Register a method service |
| `removeService(method)` | Remove a method service |
| `request(method, params, cb)` | Send a request (requires response) |
| `request(method, cb)` | Send a request (no params) |
| `notify(method, params)` | Send a notification (no response needed) |
| `notify(method)` | Send a notification (no params) |
| `respondResult(int_id, result)` | Asynchronous response with success result |
| `respondError(int_id, errcode, message)` | Asynchronous response with error |
| `clear()` | Clear cached data |

### ServiceCallback — Method Callback

```cpp
using ServiceCallback = std::function<bool(int int_id, const Json &params, Response &response)>;
```

- Return `true`: synchronous response — the function automatically responds based on response after returning
- Return `false`: asynchronous response — manually respond later via `respondResult/respondError`

### Proto — Protocol Transport Layer

Proto is the transport layer abstraction of the protocol. Users need to implement the concrete transport method (e.g., based on TcpConnection):

| Method | Description |
|--------|-------------|
| `setRecvCallback(req_cb, rsp_cb)` | Set receive callbacks |
| `setSendCallback(send_cb)` | Set send callback |
| `onRecvData(data, size)` | Process received data (must be implemented by subclass) |

## Usage Examples

### Request Side (Ping)

> Full example at `examples/jsonrpc/req_rsp/ping/`

```cpp
Rpc rpc(sp_loop, IdType::kInt);
rpc.initialize(proto, 30);  //! Timeout 30 seconds

//! Send request, wait for response
rpc.request("ping", Json::object{{"data", "hello"}},
    [](const Response &rsp) {
        if (rsp.errcode == 0)
            LogInfo("result: %s", rsp.result.dump().c_str());
        else
            LogErr("error: %d, %s", rsp.errcode, rsp.message.c_str());
    }
);
```

### Service Side (Pong)

> Full example at `examples/jsonrpc/req_rsp/pong/`

```cpp
Rpc rpc(sp_loop, IdType::kInt);
rpc.initialize(proto, 30);

//! Register service method
rpc.addService("ping",
    [](int int_id, const Json &params, Response &response) {
        //! Synchronous response
        response.errcode = 0;
        response.result = Json::object{{"echo", params["data"]}};
        return true;
    }
);
```

### Asynchronous Response

```cpp
rpc.addService("async_query",
    [](int int_id, const Json &params, Response &response) {
        //! Asynchronous processing: do not respond immediately, respond later via respondResult
        //! Return false to indicate no automatic response
        thread_pool.execute(
            [int_id, params] { /* background query */ },
            [int_id, &rpc] {
                rpc.respondResult(int_id, Json::object{{"status", "ok"}});
            }
        );
        return false;
    }
);
```

### Notification (No Response Needed)

```cpp
//! Send notification
rpc.notify("event", Json::object{{"type", "alert"}});
```

### Message Communication (Ping/Pong One-way)

> Full example at `examples/jsonrpc/message/ping/` and `pong/`

Suitable for simple message passing scenarios without the request-response pattern.

## Common Scenarios

1. **Request-Response**: Client sends a request, server responds with a result
2. **Asynchronous Processing**: Server receives a request, processes asynchronously, and responds later
3. **Event Notification**: One side sends a notification message, the other side only receives without responding
4. **Timeout Management**: Requests that exceed the timeout automatically trigger a timeout callback

## Important Notes

1. **ID Type**: Int ID auto-increment is simple and efficient; String ID (e.g., UUID) is more secure but has higher overhead
2. **Proto Implementation**: You must implement the Proto's `onRecvData()` method yourself to parse transport layer data
3. **Timeout**: Requests sent via `request` that do not receive a response within the timeout period will trigger a timeout callback (errcode != 0)
4. **int_id for Asynchronous Response**: When responding asynchronously, you must save the `int_id` and use it later to respond
5. **clear() Resets State**: Resets the RPC object's cached data, restoring it to a state where no data has been sent or received

## Related Modules

- **event**: Runs based on Loop
- **eventx**: Uses TimeoutMonitor for request timeout management
- **network**: Can implement Proto transport layer based on TcpConnection
- **base**: Provides infrastructure such as Json, Cabinet/Token
