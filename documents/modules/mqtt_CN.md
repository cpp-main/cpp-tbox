# MQTT 客户端模块 (mqtt)

## 是什么？

mqtt 模块提供了 MQTT 协议客户端实现，基于 libmosquitto 库封装，与 cpp-tbox 的事件循环无缝集成，支持 TLS 加密连接、遗言配置、自动重连等特性。

## 为什么需要它？

在 IoT 和消息中间件场景中，MQTT 是最常用的轻量级消息协议。mqtt 模块让 C++ 服务程序能方便地连接 MQTT Broker，订阅/发布主题，同时享受事件驱动的异步回调模式。

## 头文件

```cpp
#include <tbox/mqtt/client.h>
```

## 核心类与接口

### Client — MQTT 客户端

| 方法 | 说明 |
|------|------|
| `Client(loop)` | 构造 |
| `initialize(config, callbacks)` | 初始化配置和回调 |
| `start()` | 开始连接 |
| `stop()` | 停止连接 |
| `subscribe(topic, mid, qos)` | 订阅主题 |
| `unsubscribe(topic, mid)` | 取消订阅 |
| `publish(topic, payload, size, qos, retain, mid)` | 发布消息 |
| `cleanup()` | 清理 |
| `getState()` | 获取当前状态 |

### 状态流转

```
kNone → kInited → kConnecting → kTcpConnected → kMqttConnected
                                          ↓ (断连)
                                    kReconnWaiting → kConnecting (自动重连)
                                          ↓ (不需重连)
                                          kEnd
```

### Config — 配置

```cpp
mqtt::Client::Config conf;

//! 基础配置
conf.base.broker.domain = "broker.emqx.io";  //! Broker 地址
conf.base.broker.port = 1883;                //! Broker 端口
conf.base.client_id = "my_client";           //! 客户端 ID
conf.base.username = "user";                 //! 用户名
conf.base.passwd = "pass";                   //! 密码
conf.base.keepalive = 60;                    //! 心跳时长（秒）

//! TLS 配置（可选）
conf.tls.enabled = true;
conf.tls.ca_file = "./ca.pem";

//! 遗言配置（可选）
conf.will.enabled = true;
conf.will.topic = "/device/offline";
conf.will.payload = Memblock("offline", 7);

//! 自动重连配置
conf.auto_reconnect_enable = true;
conf.auto_reconnect_wait_sec_gen_func = [](int fail_count) {
    return 1 << std::min(fail_count, 4);  //! 指数退避：1, 2, 4, 8, 16...
};
```

### Callbacks — 回调函数

```cpp
mqtt::Client::Callbacks cbs;

cbs.connected = [] { LogInfo("MQTT connected"); };
cbs.connect_fail = [] { LogErr("MQTT connect fail"); };
cbs.disconnected = [] { LogInfo("MQTT disconnected"); };
cbs.message_recv = [](int mid, const std::string &topic,
                       const void *payload, int size, int qos, bool retain) {
    LogInfo("recv topic:%s, data:%.*s", topic.c_str(), size, (char*)payload);
};
cbs.message_pub = [](int mid) { LogInfo("published, mid=%d", mid); };
cbs.subscribed = [](int mid, int qos, const int *granted_qos) { LogInfo("subscribed"); };
cbs.unsubscribed = [](int mid) { LogInfo("unsubscribed"); };
cbs.state_changed = [](mqtt::Client::State state) { LogInfo("state: %d", state); };
```

## 使用示例

### 连接 MQTT Broker

> 完整示例见 `examples/mqtt/conn/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/mqtt/client.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>

using namespace tbox;
using namespace tbox::event;

int main() {
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    SetScopeExitAction([sp_loop] { delete sp_loop; });

    mqtt::Client mqtt(sp_loop);

    mqtt::Client::Config conf;
    conf.auto_reconnect_enable = true;
    conf.auto_reconnect_wait_sec_gen_func = [](int fail_count) {
        return 1 << std::min(fail_count, 4);  //! 指数退避重连
    };

    if (!mqtt.initialize(conf, mqtt::Client::Callbacks())) {
        LogErr("init mqtt fail");
        return 0;
    }

    mqtt.start();

    //! 监听退出信号
    auto stop_ev = sp_loop->newSignalEvent();
    SetScopeExitAction([stop_ev] { delete stop_ev; });
    stop_ev->initialize(SIGINT, Event::Mode::kOneshot);
    stop_ev->enable();
    stop_ev->setCallback([sp_loop, &mqtt] (int) {
        mqtt.stop();
        sp_loop->exitLoop();
    });

    sp_loop->runLoop(Loop::Mode::kForever);
    mqtt.cleanup();

    LogOutput_Disable();
    return 0;
}
```

### 订阅主题

> 完整示例见 `examples/mqtt/sub/`

```cpp
mqtt::Client::Callbacks cbs;
cbs.connected = [&mqtt] {
    LogInfo("connected, subscribing...");
    mqtt.subscribe("/sensor/temperature", nullptr, 1);  //! QoS=1
};
cbs.message_recv = [](int mid, const std::string &topic,
                       const void *payload, int size, int qos, bool retain) {
    LogInfo("topic:%s, payload:%.*s", topic.c_str(), size, (const char*)payload);
};

mqtt.initialize(conf, cbs);
```

### 发布消息

> 完整示例见 `examples/mqtt/pub/`

```cpp
mqtt::Client::Callbacks cbs;
cbs.connected = [&mqtt] {
    LogInfo("connected, publishing...");
    mqtt.publish("/sensor/temperature", "25.6", 4, 1, false);  //! QoS=1, retain=false
};

mqtt.initialize(conf, cbs);
```

## 常见场景

1. **IoT 数据上报**：设备连接 Broker，定期 publish 传感器数据
2. **消息订阅**：服务端 subscribe 主题，接收设备上报数据
3. **设备状态监控**：利用遗言（Will）机制，设备离线时自动发布离线消息
4. **TLS 安全连接**：启用 TLS 加密，适用于安全要求高的场景
5. **断线自动重连**：启用 auto_reconnect，配合指数退避策略

## 注意事项

1. **自动重连策略**：建议使用指数退避（`1 << min(fail_count, 4)`），避免频繁重连消耗资源
2. **Memblock 遗言**：遗言 payload 使用 `Memblock` 类型，支持二进制数据
3. **回调在 Loop 线程执行**：所有回调在主 Loop 线程中执行，不要在回调中做耗时操作
4. **初始化顺序**：先 initialize()，再 start()；stop() 后才能 cleanup()
5. **依赖 libmosquitto**：编译时需要链接 mosquitto 库

## 相关模块

- **event**：基于 Loop 实现 socket 读写和定时器
- **base**：提供 Memblock、Log 宏等
- **network**：使用 SocketFd/TcpConnection 实现底层连接
