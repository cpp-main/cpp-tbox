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

#ifndef TBOX_DBUS_SETUP_WITH_LOOP_H_20230729
#define TBOX_DBUS_SETUP_WITH_LOOP_H_20230729

#include <tbox/event/loop.h>
#include <dbus/dbus.h>

namespace tbox {
namespace dbus {

/**
 * 将 DBusConnection 对象挂载到 event::Loop 上
 *
 * \param dbus_conn     DBusConnection 对象指针
 * \param loop          event::Loop 对象指针
 */
void AttachLoop(DBusConnection *dbus_conn, event::Loop *loop);

/**
 * 将 DBusConnection 对象从 event::Loop 上卸载
 *
 * \param dbus_conn     DBusConnection 对象指针
 */
void DetachLoop(DBusConnection *dbus_conn);

}
}

#endif //TBOX_DBUS_SETUP_WITH_LOOP_H_20230729
