#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/event/loop.h>
#include <tbox/event/signal_event.h>
#include <tbox/event/timer_event.h>
#include "tbox/dbus/setup_with_loop.h"

tbox::event::Loop *g_loop = nullptr;
DBusConnection *g_dbus_conn = nullptr;

//! 处理接收到的消息
DBusHandlerResult MessageFilter(DBusConnection *connection, DBusMessage *message, void *data) {
  if (dbus_message_is_signal(message, "test.signal.Type", "Test")) {
    DBusMessageIter msg_args_iter;
    char* sigvalue;

    if (!dbus_message_iter_init(message, &msg_args_iter)) {
      LogWarn("Message Init Failed");
      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&msg_args_iter)) {
      LogNotice("Arg is not string!");
      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    } else {
      dbus_message_iter_get_basic(&msg_args_iter, &sigvalue);
    }

    LogDbg("Got Signal with value: '%s'", sigvalue);
    return DBUS_HANDLER_RESULT_HANDLED;
  }

  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

bool InitDbus() {
  DBusError dbus_error = DBUS_ERROR_INIT;
  g_dbus_conn = dbus_bus_get_private(DBUS_BUS_SESSION, &dbus_error);
  if (g_dbus_conn == nullptr) {
    if (dbus_error_is_set(&dbus_error)) {
      LogErr("open dbus fail, %s", dbus_error.message);
      dbus_error_free(&dbus_error);
      return false;
    } else {
      LogErr("open dbus fail");
      return false;
    }
  }

  dbus_bus_request_name(g_dbus_conn, "my.dbus.test", DBUS_NAME_FLAG_DO_NOT_QUEUE, &dbus_error);
  if (dbus_error_is_set(&dbus_error)) {
    LogErr("request name fail, %s", dbus_error.message);
    dbus_error_free(&dbus_error);
  }

  tbox::dbus::SetupWithLoop(g_dbus_conn, g_loop);

  dbus_connection_add_filter(g_dbus_conn, MessageFilter, nullptr, nullptr);
  dbus_connection_set_exit_on_disconnect(g_dbus_conn, false);
  return true;
}

void  DesinitDbus() {
  dbus_connection_close(g_dbus_conn);
  dbus_connection_unref(g_dbus_conn);
}

void  AddWatchs() {
  DBusError dbus_error = DBUS_ERROR_INIT;
  dbus_bus_add_match(g_dbus_conn,"type='signal',path='/test/signal/Object',interface='test.signal.Type'",&dbus_error);
  if (dbus_error_is_set(&dbus_error)) {
    LogErr("open dbus fail, %s", dbus_error.message);
    dbus_error_free(&dbus_error);
  }
}

void SendSignal() {
  LogTag();
  dbus_uint32_t seq = 0;
  DBusMessage *bus_msg;
  DBusMessageIter msg_args_iter;
  void *sigvalue = (void *)"hello tbox dbus";

  bus_msg = dbus_message_new_signal("/test/signal/Object", "test.signal.Type", "Test");
  if (bus_msg == nullptr) {
    LogErr("new message fail");
    return;
  }
  dbus_message_iter_init_append(bus_msg, &msg_args_iter);
  if (!dbus_message_iter_append_basic(&msg_args_iter, DBUS_TYPE_STRING, &sigvalue)) {
    LogErr("fail");
    return;
  }

  if (!dbus_connection_send(g_dbus_conn, bus_msg, &seq)) {
    LogErr("fail");
    return;
  }
  LogDbg("seq:%u", seq);
}

int main() {
  LogOutput_Enable();

  g_loop = tbox::event::Loop::New();

  if (!InitDbus())
    return 0;

  AddWatchs();

  auto timer = g_loop->newTimerEvent();
  timer->initialize(std::chrono::seconds(5), tbox::event::Event::Mode::kPersist);
  timer->setCallback(SendSignal);
  timer->enable();

  auto signal = g_loop->newSignalEvent();
  signal->initialize(SIGINT, tbox::event::Event::Mode::kOneshot);
  signal->setCallback([](int) { g_loop->exitLoop(); });
  signal->enable();

  LogInfo("Start");
  g_loop->runLoop();
  LogInfo("Stop");

  DesinitDbus();

  delete signal;
  delete g_loop;
  return 0;
}

