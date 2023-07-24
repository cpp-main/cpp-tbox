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
#include "session.h"
#include <tbox/base/log.h>

#include "connection.h"

namespace tbox {
namespace terminal {

Session::Session(Connection *wp_conn, const SessionToken &st) :
    wp_conn_(wp_conn),
    st_(st)
{ }

bool Session::send(char ch) const
{
    return wp_conn_->send(st_, ch);
}

bool Session::send(const std::string &str) const
{
    return wp_conn_->send(st_, str);
}

bool Session::endSession() const
{
    return wp_conn_->endSession(st_);
}

bool Session::isValid() const
{
    return wp_conn_->isValid(st_);
}

}
}
