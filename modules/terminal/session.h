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
#ifndef TBOX_TERMINAL_SESSION_H_20220214
#define TBOX_TERMINAL_SESSION_H_20220214

#include "types.h"

namespace tbox {
namespace terminal {

class Connection;

class Session {
  public:
    Session(Connection *wp_conn, const SessionToken &st);

    bool send(char ch) const;
    bool send(const std::string &str) const;
    bool endSession() const;
    bool isValid() const;

  private:
    Connection *wp_conn_;
    const SessionToken st_;
};

}
}

#endif //TBOX_TERMINAL_SESSION_H_20220214
