# D-Bus Integration Module (dbus)

## What is it?

The dbus module provides integration between D-Bus (the Linux desktop/system inter-process communication bus) and the cpp-tbox event loop, enabling tbox-based service applications to interact with other system services via D-Bus.

## Why do you need it?

In Linux systems, D-Bus is the standard mechanism for inter-process communication. Many system services (such as systemd, NetworkManager, Bluetooth services, etc.) expose interfaces through D-Bus. The dbus module allows tbox programs to interact with these services while maintaining an event-driven asynchronous model.

## Header Files

```cpp
#include <tbox/dbus/loop.h>          //! D-Bus integration with event::Loop
#include <tbox/dbus/connection.h>     //! D-Bus connection
```

## Core Classes and Interfaces

### Connection — D-Bus Connection

| Method | Description |
|------|------|
| `Connection(loop)` | Constructor, specifies the event loop |
| `initialize(bus_type)` | Initialize, specifies the bus type (kSession/kSystem/kStarter) |
| `initialize(bus_address)` | Initialize, specifies a custom bus address |
| `cleanup()` | Clean up the connection |

Bus types:
- `kSession` — Session bus (user-level desktop services)
- `kSystem` — System bus (system-level services)
- `kStarter` — Starter bus

### AttachLoop / DetachLoop — Attach/Detach Loop

```cpp
//! Attach an existing DBusConnection object to event::Loop
dbus::AttachLoop(dbus_conn, sp_loop);

//! Detach from event::Loop
dbus::DetachLoop(dbus_conn);
```

Useful for scenarios where you already have a DBusConnection (such as a connection object obtained directly from libdbus).

## Usage Examples

### Basic Connection

> Full example available in `examples/dbus/00-loop/`

```cpp
#include <tbox/event/loop.h>
#include <tbox/dbus/connection.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

using namespace tbox;
using namespace tbox::event;

int main() {
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();

    dbus::Connection dbus_conn(sp_loop);
    dbus_conn.initialize(dbus::Connection::kSession);  //! Connect to session bus

    //! From here, dbus_conn can be used for D-Bus method calls, signal reception, etc.

    sp_loop->runLoop();
    dbus_conn.cleanup();

    LogOutput_Disable();
    return 0;
}
```

### Attaching an Existing DBusConnection

```cpp
DBusConnection *raw_conn = dbus_bus_get(DBUS_BUS_SESSION, nullptr);

//! Integrate the native D-Bus connection into the tbox event loop
dbus::AttachLoop(raw_conn, sp_loop);

//! D-Bus event listening is now managed by the tbox Loop

//! Detach
dbus::DetachLoop(raw_conn);
```

## Common Scenarios

1. **Interacting with System Services**: Call NetworkManager, systemd and other system service interfaces via D-Bus
2. **Desktop Application Integration**: Communicate with GNOME/KDE desktop services
3. **Cross-process Signal Delivery**: Pass events between processes using the D-Bus signal mechanism
4. **Hardware Status Queries**: Query Bluetooth, USB device and other hardware status

## Important Notes

1. **Dependency on libdbus**: The dbus-1 library must be linked at compile time
2. **Linux Only**: D-Bus is a Linux-specific IPC mechanism and cannot be used on other platforms
3. **Connection Encapsulation**: The Connection object encapsulates DBusConnection creation and event integration, no manual management needed
4. **Event-driven**: D-Bus event listening is managed by Loop, no additional dbus_watch/dbus_timeout handling required
5. **Cleanup Order**: Clean up Connection before cleaning up Loop when the program exits

## Related Modules

- **event**: Manages D-Bus event listening based on Loop
- **base**: Provides Log and other infrastructure
