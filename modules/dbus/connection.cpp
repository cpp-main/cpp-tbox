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

#include "connection.h"

#include <tbox/base/log.h>
#include "loop.h"

namespace tbox {
namespace dbus {

Connection::Connection(event::Loop *loop)
    : loop_(loop)
{
    LogUndo();
}

Connection::~Connection() {
    LogUndo();
}

bool Connection::initialize(BusType bus_type)
{
    ::DBusError dbus_error = DBUS_ERROR_INIT;
    dbus_conn_ = ::dbus_bus_get_private(static_cast<DBusBusType>(bus_type), &dbus_error);
    if (dbus_conn_ == nullptr) {
        if (::dbus_error_is_set(&dbus_error)) {
            LogErr("open dbus fail, %s", dbus_error.message);
            ::dbus_error_free(&dbus_error);
        } else {
            LogErr("open dbus fail");
        }
        return false;
    }

    AttachLoop(dbus_conn_, loop_);
    return true;
   
}

bool Connection::initialize(const std::string bus_address)
{
    ::DBusError dbus_error = DBUS_ERROR_INIT;
    dbus_conn_ = ::dbus_connection_open_private(bus_address.c_str(), &dbus_error);
    if (dbus_conn_ == nullptr) {
        if (::dbus_error_is_set(&dbus_error)) {
            LogErr("open dbus %s fail, %s", bus_address.c_str(), dbus_error.message);
            ::dbus_error_free(&dbus_error);
        } else {
            LogErr("open dbus %s fail, no error", bus_address.c_str());
        }
        return false;
    }

    AttachLoop(dbus_conn_, loop_);
    return true;
}

void Connection::cleanup()
{
    ::dbus_connection_close(dbus_conn_);
    DetachLoop(dbus_conn_);

    ::dbus_connection_unref(dbus_conn_);
}

}
}
