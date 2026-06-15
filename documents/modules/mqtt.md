# MQTT Client Module (mqtt)

## What is it?

The mqtt module provides an MQTT protocol client implementation, built on top of the libmosquitto library and seamlessly integrated with cpp-tbox's event loop. It supports TLS encrypted connections, will message configuration, automatic reconnection, and more.

## Why do you need it?

In IoT and message middleware scenarios, MQTT is the most commonly used lightweight messaging protocol. The mqtt module enables C++ service programs to easily connect to an MQTT Broker, subscribe to/publish topics, while enjoying the event-driven asynchronous callback model.

## Header Files

```cpp
#include <tbox/mqtt/client.h>
```

## Core Classes and Interfaces

### Client — MQTT Client

| Method | Description |
|------|------|
| `Client(loop)` | Constructor |
| `initialize(config, callbacks)` | Initialize configuration and callbacks |
| `start()` | Start connection |
| `stop()` | Stop connection |
| `subscribe(topic, mid, qos)` | Subscribe to a topic |
| `unsubscribe(topic, mid)` | Unsubscribe from a topic |
| `publish(topic, payload, size, qos, retain, mid)` | Publish a message |
| `cleanup()` | Cleanup |
| `getState()` | Get current state |

### State Transitions

```
kNone → kInited → kConnecting → kTcpConnected → kMqttConnected
                                        ↓ (disconnected)
                                  kReconnWaiting → kConnecting (auto reconnect)
                                        ↓ (no reconnect needed)
                                        kEnd
```

### Config — Configuration

```cpp
mqtt::Client::Config conf;

//! Basic configuration
conf.base.broker.domain = "broker.emqx.io";  //! Broker address
conf.base.broker.port = 1883;                //! Broker port
conf.base.client_id = "my_client";           //! Client ID
conf.base.username = "user";                 //! Username
conf.base.passwd = "pass";                   //! Password
conf.base.keepalive = 60;                    //! Keepalive interval (seconds)

//! TLS configuration (optional)
conf.tls.enabled = true;
conf.tls.ca_file = "./ca.pem";

//! Will message configuration (optional)
conf.will.enabled = true;
conf.will.topic = "/device/offline";
conf.will.payload = Memblock("offline", 7);

//! Auto reconnect configuration
conf.auto_reconnect_enable = true;
conf.auto_reconnect_wait_sec_gen_func = [](int fail_count) {
    return 1 << std::min(fail_count, 4);  //! Exponential backoff: 1, 2, 4, 8, 16...
};
```

### Callbacks — Callback Functions

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

## Usage Examples

### Connecting to an MQTT Broker

> Full example in `examples/mqtt/conn/`

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
        return 1 << std::min(fail_count, 4);  //! Exponential backoff reconnect
    };

    if (!mqtt.initialize(conf, mqtt::Client::Callbacks())) {
        LogErr("init mqtt fail");
        return 0;
    }

    mqtt.start();

    //! Listen for exit signal
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

### Subscribing to a Topic

> Full example in `examples/mqtt/sub/`

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

### Publishing a Message

> Full example in `examples/mqtt/pub/`

```cpp
mqtt::Client::Callbacks cbs;
cbs.connected = [&mqtt] {
    LogInfo("connected, publishing...");
    mqtt.publish("/sensor/temperature", "25.6", 4, 1, false);  //! QoS=1, retain=false
};

mqtt.initialize(conf, cbs);
```

## Common Scenarios

1. **IoT data reporting**: Devices connect to the Broker and periodically publish sensor data
2. **Message subscription**: Server-side subscribes to topics to receive data reported by devices
3. **Device status monitoring**: Using the Will message mechanism, devices automatically publish offline messages when disconnected
4. **TLS secure connection**: Enable TLS encryption for scenarios with high security requirements
5. **Automatic reconnection on disconnection**: Enable auto_reconnect with an exponential backoff strategy

## Important Notes

1. **Auto reconnect strategy**: Use exponential backoff (`1 << min(fail_count, 4)`) to avoid frequent reconnects that waste resources
2. **Memblock will message**: Will message payload uses the `Memblock` type, which supports binary data
3. **Callbacks run in the Loop thread**: All callbacks execute in the main Loop thread — do not perform time-consuming operations in callbacks
4. **Initialization order**: Call initialize() first, then start(); call cleanup() only after stop()
5. **Depends on libmosquitto**: The mosquitto library must be linked at compile time

## Related Modules

- **event**: Implements socket I/O and timers based on Loop
- **base**: Provides Memblock, Log macros, etc.
- **network**: Implements underlying connections using SocketFd/TcpConnection
