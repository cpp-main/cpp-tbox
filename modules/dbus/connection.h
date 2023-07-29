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

#ifndef TBOX_DBUS_CONNECTION_H_20230729
#define TBOX_DBUS_CONNECTION_H_20230729

#include <tbox/event/loop.h>
#include <dbus/dbus.h>

namespace tbox {
namespace dbus {

class Connection {
  public:
    explicit Connection(event::Loop *loop);
    virtual ~Connection();

  public:
    enum BusType {
        kSession = 0,
        kSystem  = 1,
        kStarter = 2,
    };

    bool initialize(BusType bus_type);
    bool initialize(const std::string bus_address);

    void cleanup();

  private:
    event::Loop *loop_;
    DBusConnection *dbus_conn_ = nullptr;
};

}
}

#endif //TBOX_DBUS_CONNECTION_H_20230729
