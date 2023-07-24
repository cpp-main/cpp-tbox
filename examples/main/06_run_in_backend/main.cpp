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
#include <tbox/main/main.h>
#include <iostream>
#include <thread>
#include <signal.h>

bool keep_running = true;

void SignalProc(int signo) {
  keep_running = false;
}

void MainLoop() {
  signal(SIGINT, SignalProc);
  signal(SIGTERM, SignalProc);

  while (keep_running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    std::cout << '.' << std::flush;
  }
}

int main(int argc, char **argv) {
  if (!tbox::main::Start(argc, argv))
    return 0;

  MainLoop();
  tbox::main::Stop();

  return 0;
}
