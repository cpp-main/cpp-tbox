/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

#include "setup_with_loop.h"

#include <tbox/base/log.h>
#include <tbox/event/fd_event.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace dbus {

namespace {

class Context {
 public:
  Context(event::Loop *loop, DBusConnection *dbus_conn)
    : loop_(loop), dbus_conn_(dbus_conn)
  {
    LogTag();
    ::dbus_connection_ref(dbus_conn_);
  }

  ~Context()
  {
    LogTag();
    ::dbus_connection_unref(dbus_conn_);
  }

  inline event::Loop* loop() const { return loop_; }

  inline DBusConnection* dbus_conn() const { return dbus_conn_; }

 private:
  event::Loop *loop_;
  DBusConnection *dbus_conn_;
};

void ContextDeleter(void *p)
{
  LogTag();
  auto ctx = static_cast<Context*>(p);
  delete ctx;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void _QueueDispatch(Context *ctx, DBusDispatchStatus status)
{
  LogTag();
  if (status == DBUS_DISPATCH_DATA_REMAINS) {
    auto dbus_conn = ctx->dbus_conn();
    dbus_connection_ref(dbus_conn);
    ctx->loop()->runNext(
      [dbus_conn] {
        while (dbus_connection_dispatch(dbus_conn) == DBUS_DISPATCH_DATA_REMAINS) ;
        dbus_connection_unref(dbus_conn);
      }
    );
  }
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////

class WatchHandler {
  public:
    WatchHandler(Context *ctx, DBusWatch *dbus_watch)
      : ctx_(ctx)
      , dbus_watch_(dbus_watch)
      , tbox_fd_(ctx->loop()->newFdEvent())
    {
      LogTag();
      auto fd = ::dbus_watch_get_unix_fd(dbus_watch);
      auto dbus_flags = ::dbus_watch_get_flags(dbus_watch);
      LogTrace("fd:%d, dbus_flags:%04x", fd, dbus_flags);

      short tbox_flags = 0;
      if (dbus_flags & DBUS_WATCH_READABLE)
        tbox_flags |= tbox::event::FdEvent::kReadEvent;
      if (dbus_flags & DBUS_WATCH_WRITABLE)
        tbox_flags |= tbox::event::FdEvent::kWriteEvent;

      tbox_fd_->initialize(fd, tbox_flags, event::Event::Mode::kPersist); //!FIXME: 可能有异常
      tbox_fd_->setCallback([this](short events) { onWatch(events); });
      tbox_fd_->enable();
    }

    ~WatchHandler()
    {
      LogTag();
      tbox_fd_->disable();
      delete tbox_fd_;
    }

  protected:
    void onWatch(short events)
    {
      LogTag();
      unsigned int dbus_flags = 0;
      if (events & tbox::event::FdEvent::kReadEvent)
        dbus_flags |= DBUS_WATCH_READABLE;

      if (events & tbox::event::FdEvent::kWriteEvent)
        dbus_flags |= DBUS_WATCH_WRITABLE;

      dbus_watch_handle(dbus_watch_, dbus_flags);

      dbus_connection_ref(ctx_->dbus_conn());
      auto status = dbus_connection_get_dispatch_status(ctx_->dbus_conn());
      _QueueDispatch(ctx_, status);
      dbus_connection_unref(ctx_->dbus_conn());
    }

  private:
    Context *ctx_;
    DBusWatch *dbus_watch_;
    event::FdEvent *tbox_fd_;
};

void _WatchHandlerDeleter(void *p)
{
  LogTag();
  auto handler = static_cast<WatchHandler*>(p);
  delete handler;
}

dbus_bool_t _AddWatch(DBusWatch *watch, void *data) {
  LogTag();
  auto ctx = static_cast<Context*>(data);

  if (::dbus_watch_get_enabled(watch))
      return true;

  LogTag();
  auto handler = new WatchHandler(ctx, watch);
  ::dbus_watch_set_data(watch, handler, _WatchHandlerDeleter);

  return true;
}

void _RemoveWatch(DBusWatch *watch, void *data) {
  LogTag();
  ::dbus_watch_set_data(watch, nullptr, nullptr);
}

void _ToggledWatch(DBusWatch *watch, void *data) {
  LogTag();
  if (::dbus_watch_get_enabled(watch))
    _AddWatch(watch, data);
  else
    _RemoveWatch(watch, data);
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////

class TimeoutHandler {
  public:
    TimeoutHandler(event::Loop *loop, DBusTimeout *dbus_timeout)
      : tbox_timer_(loop->newTimerEvent())
      , dbus_timeout_(dbus_timeout_)
    {
      LogTag();
      int interval_ms = ::dbus_timeout_get_interval(dbus_timeout);
      //tbox_timer_->initialize(std::chrono::milliseconds(interval_ms), event::Event::Mode::kOneshot);  //! FIXME: 可能有异常
      tbox_timer_->initialize(std::chrono::milliseconds(interval_ms), event::Event::Mode::kPersist);
      tbox_timer_->setCallback([this] { onTimeout(); });
      tbox_timer_->enable();
    }

    ~TimeoutHandler()
    {
      LogTag();
      tbox_timer_->disable();
      delete tbox_timer_;
    }

  protected:
    void onTimeout()
    {
      LogTag();
      if (dbus_timeout_get_enabled(dbus_timeout_)) {
        dbus_timeout_handle(dbus_timeout_);
      }
    }
  private:
    event::TimerEvent *tbox_timer_;
    DBusTimeout *dbus_timeout_;
};

void _TimeoutHandlerDeleter(void *p)
{
  LogTag();
  auto handler = static_cast<TimeoutHandler*>(p);
  delete handler;
}

dbus_bool_t _AddTimeout(DBusTimeout *timeout, void *data)
{
  LogTag();
  auto ctx = static_cast<Context*>(data);

  if (::dbus_timeout_get_enabled(timeout))
      return true;

  auto handler = new TimeoutHandler(ctx->loop(), timeout);
  ::dbus_timeout_set_data(timeout, handler, _TimeoutHandlerDeleter);

  return true;
}

void _RemoveTimeout(DBusTimeout *timeout, void *)
{
  LogTag();
  ::dbus_timeout_set_data(timeout, nullptr, nullptr);
}

void _ToggledTimeout(DBusTimeout *timeout, void *data)
{
  LogTag();
  if (::dbus_timeout_get_enabled(timeout))
    _AddTimeout(timeout, data);
  else
    _RemoveTimeout(timeout, data);
}

//////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////

void _DispatchStatus(DBusConnection *conn, DBusDispatchStatus status, void *data)
{
  LogTag();
  if (dbus_connection_get_is_connected(conn)) {
    LogTag();
    Context *ctx = static_cast<Context*>(data);
    _QueueDispatch(ctx, status);
  }
}

}

void SetupWithLoop(DBusConnection *dbus_conn, event::Loop *loop)
{
    LogTag();
    auto watch_ctx = new Context(loop, dbus_conn);
    ::dbus_connection_set_watch_functions(
        dbus_conn,
        _AddWatch, _RemoveWatch, _ToggledWatch,
        watch_ctx, ContextDeleter);

    auto timeout_ctx = new Context(loop, dbus_conn);
    ::dbus_connection_set_timeout_functions(
        dbus_conn,
        _AddTimeout, _RemoveTimeout, _ToggledTimeout,
        timeout_ctx, ContextDeleter);

    auto dispatch_ctx = new Context(loop, dbus_conn);
    ::dbus_connection_set_dispatch_status_function(
        dbus_conn,
        _DispatchStatus,
        dispatch_ctx, ContextDeleter);
}

}
}
