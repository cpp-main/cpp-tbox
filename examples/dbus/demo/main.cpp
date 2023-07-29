#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/dbus/connection.h>

int main() {
  LogOutput_Enable();

  auto loop = tbox::event::Loop::New();
  auto signal = loop->newSignalEvent();

  tbox::dbus::Connection dbus_conn(loop);
  dbus_conn.initialize(tbox::dbus::Connection::BusType::kSession);

  signal->initialize(SIGINT, tbox::event::Event::Mode::kOneshot);
  signal->setCallback([loop](int) { loop->exitLoop(); });
  signal->enable();

  LogInfo("Start");
  loop->runLoop();
  LogInfo("Stop");

  dbus_conn.cleanup();

  delete signal;
  delete loop;
  return 0;
}
