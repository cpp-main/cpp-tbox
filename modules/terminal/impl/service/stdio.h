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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_TERMINAL_STDIO_IMP_H_20250325
#define TBOX_TERMINAL_STDIO_IMP_H_20250325

#include <termios.h>

#include <map>
#include <tbox/network/stdio_stream.h>

#include "../../connection.h"
#include "../../service/stdio.h"
#include "../../terminal_interact.h"

namespace tbox {
namespace terminal {

using namespace std;
using namespace event;
using namespace network;

class Stdio::Impl : Connection {
  public:
    Impl(Loop *wp_loop, TerminalInteract *wp_terminal);

  public:
    bool initialize();
    bool start();
    void stop();
    void cleanup();

  public:
    virtual bool send(const SessionToken &st, char ch) override;
    virtual bool send(const SessionToken &st, const std::string &str) override;
    virtual bool endSession(const SessionToken &st) override;
    virtual bool isValid(const SessionToken &st) const override;

  protected:
    void onStdinRecv(util::Buffer& buff);

    void disableTermiosBuffer();
    void restoreTermiosBuffer();

    void startSession();

  private:
    Loop *wp_loop_ = nullptr;
    TerminalInteract *wp_terminal_ = nullptr;
    StdioStream stdio_stream_;
    struct termios origin_settings_;

    SessionToken session_token_;
};

}
}

#endif //TBOX_TERMINAL_STDIO_IMP_H_20250325
