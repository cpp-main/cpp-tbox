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
#ifndef TBOX_TERMINAL_STDIO_H_20250325
#define TBOX_TERMINAL_STDIO_H_20250325

#include <tbox/event/loop.h>

namespace tbox {
namespace terminal {

class TerminalInteract;

class Stdio {
  public:
    Stdio(event::Loop *wp_loop, TerminalInteract *wp_terminal);
    ~Stdio();

  public:
    bool initialize();
    bool start();
    void stop();
    void cleanup();

  private:
    class Impl;
    Impl *impl_ = nullptr;
};

}
}

#endif //TBOX_TERMINAL_STDIO_H_20250325
