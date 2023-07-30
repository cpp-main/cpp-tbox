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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

#include "setup_with_loop.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
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
      ::dbus_connection_ref(dbus_conn_);
  }

  ~Context()
  {
      ::dbus_connection_unref(dbus_conn_);
  }

  inline event::Loop* loop() const { return loop_; }
  inline DBusConnection* dbus_conn() const { return dbus_conn_; }

 private:
  event::Loop *loop_;
  DBusConnection *dbus_conn_;
};

void _ContextDeleter(void *p)
{
    auto ctx = static_cast<Context*>(p);
    delete ctx;
}

//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
void _QueueDispatch(Context *ctx, DBusDispatchStatus status)
{
    if (status == DBUS_DISPATCH_DATA_REMAINS) {
        auto dbus_conn = ctx->dbus_conn();
        dbus_connection_ref(dbus_conn);
        ctx->loop()->runNext(
            [dbus_conn] {
                while (dbus_connection_dispatch(dbus_conn) == DBUS_DISPATCH_DATA_REMAINS);
                dbus_connection_unref(dbus_conn);
            }
        );
    }
}

//////////////////////////////////////////////////////////////////////////////
// 文件读写事件
//////////////////////////////////////////////////////////////////////////////

class WatchHandler {
  public:
    WatchHandler(Context *ctx, DBusWatch *dbus_watch)
      : ctx_(ctx)
      , dbus_watch_(dbus_watch)
      , tbox_fd_(ctx->loop()->newFdEvent())
    {
        auto fd = ::dbus_watch_get_unix_fd(dbus_watch);
        auto dbus_flags = ::dbus_watch_get_flags(dbus_watch);

        //! 仅允许读事件或写事件中二选一，不能同时两种事件
        TBOX_ASSERT((dbus_flags & (dbus_flags - 1)) == 0);

        short tbox_flags = 0;
        event::Event::Mode mode;

        if (dbus_flags & DBUS_WATCH_READABLE) {
            tbox_flags |= tbox::event::FdEvent::kReadEvent;
            mode = event::Event::Mode::kPersist;
        }

        if (dbus_flags & DBUS_WATCH_WRITABLE) {
            tbox_flags |= tbox::event::FdEvent::kWriteEvent;
            mode = event::Event::Mode::kOneshot;
        }

        tbox_fd_->initialize(fd, tbox_flags, mode);
        tbox_fd_->setCallback([this](short events) { onWatch(events); });
    }

    ~WatchHandler()
    {
        delete tbox_fd_;
    }

    void enable() { tbox_fd_->enable(); }
    void disable() { tbox_fd_->disable(); }

  protected:
    void onWatch(short events)
    {
        unsigned int dbus_flags = 0;
        if (events & tbox::event::FdEvent::kReadEvent)
            dbus_flags |= DBUS_WATCH_READABLE;
        if (events & tbox::event::FdEvent::kWriteEvent)
            dbus_flags |= DBUS_WATCH_WRITABLE;
        if (events & tbox::event::FdEvent::kExceptEvent)
            dbus_flags |= DBUS_WATCH_ERROR;

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
    auto handler = static_cast<WatchHandler*>(p);
    TBOX_ASSERT(handler != nullptr);
    delete handler;
}

dbus_bool_t _AddWatch(DBusWatch *watch, void *data)
{
    auto handler = static_cast<WatchHandler*>(::dbus_watch_get_data(watch));
    if (handler == nullptr) {
        auto ctx = static_cast<Context*>(data);
        handler = new WatchHandler(ctx, watch);
        ::dbus_watch_set_data(watch, handler, _WatchHandlerDeleter);
    }

    handler->enable();
    return true;
}

void _RemoveWatch(DBusWatch *watch, void *data)
{
    auto handler = static_cast<WatchHandler*>(::dbus_watch_get_data(watch));
    TBOX_ASSERT(handler != nullptr);

    handler->disable();
}

void _ToggledWatch(DBusWatch *watch, void *data)
{
    auto handler = static_cast<WatchHandler*>(::dbus_watch_get_data(watch));
    TBOX_ASSERT(handler != nullptr);

    if (::dbus_watch_get_enabled(watch))
        handler->enable();
    else
        handler->disable();
}

//////////////////////////////////////////////////////////////////////
// 超时事件
//////////////////////////////////////////////////////////////////////

class TimeoutHandler {
  public:
    TimeoutHandler(event::Loop *loop, DBusTimeout *dbus_timeout)
      : tbox_timer_(loop->newTimerEvent())
      , dbus_timeout_(dbus_timeout)
    {
        int interval_ms = ::dbus_timeout_get_interval(dbus_timeout);
        tbox_timer_->initialize(std::chrono::milliseconds(interval_ms), event::Event::Mode::kOneshot);
        tbox_timer_->setCallback([this] { onTimeout(); });
    }

    ~TimeoutHandler()
    {
        tbox_timer_->disable();
        delete tbox_timer_;
    }

    void enable() { tbox_timer_->enable(); }
    void disable() { tbox_timer_->disable(); }

  protected:
    void onTimeout()
    {
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
    auto handler = static_cast<TimeoutHandler*>(p);
    TBOX_ASSERT(handler != nullptr);
    delete handler;
}

dbus_bool_t _AddTimeout(DBusTimeout *timeout, void *data)
{
    auto handler = static_cast<TimeoutHandler*>(::dbus_timeout_get_data(timeout));
    if (handler == nullptr) {
        auto ctx = static_cast<Context*>(data);
        handler = new TimeoutHandler(ctx->loop(), timeout);
        ::dbus_timeout_set_data(timeout, handler, _TimeoutHandlerDeleter);
    }

    handler->enable();
    return true;
}

void _RemoveTimeout(DBusTimeout *timeout, void *)
{
    auto handler = static_cast<TimeoutHandler*>(::dbus_timeout_get_data(timeout));
    TBOX_ASSERT(handler != nullptr);

    handler->disable();
}

void _ToggledTimeout(DBusTimeout *timeout, void *)
{
    auto handler = static_cast<TimeoutHandler*>(::dbus_timeout_get_data(timeout));
    TBOX_ASSERT(handler != nullptr);

    if (::dbus_timeout_get_enabled(timeout))
        handler->enable();
    else
        handler->disable();
}

//////////////////////////////////////////////////////////////////////
// 异步派发动作
//////////////////////////////////////////////////////////////////////

void _DispatchStatus(DBusConnection *conn, DBusDispatchStatus status, void *data)
{
    if (dbus_connection_get_is_connected(conn)) {
        Context *ctx = static_cast<Context*>(data);
        _QueueDispatch(ctx, status);
    }
}

}

void SetupWithLoop(DBusConnection *dbus_conn, event::Loop *loop)
{
    TBOX_ASSERT(dbus_conn != nullptr);
    TBOX_ASSERT(loop != nullptr);

    //! 注册文件读写事件的创建与删除方法函数与数据
    auto watch_ctx = new Context(loop, dbus_conn);
    ::dbus_connection_set_watch_functions(dbus_conn,
            _AddWatch, _RemoveWatch, _ToggledWatch,
            watch_ctx, _ContextDeleter);

    //! 注册超时事件的创建与删除方法函数与数据
    auto timeout_ctx = new Context(loop, dbus_conn);
    ::dbus_connection_set_timeout_functions(dbus_conn,
            _AddTimeout, _RemoveTimeout, _ToggledTimeout,
            timeout_ctx, _ContextDeleter);

    //! 注册异步派发动作的方法函数与数据
    auto dispatch_ctx = new Context(loop, dbus_conn);
    ::dbus_connection_set_dispatch_status_function(dbus_conn,
            _DispatchStatus, dispatch_ctx, _ContextDeleter);
}

}
}
