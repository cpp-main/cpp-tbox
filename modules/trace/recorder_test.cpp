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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <gtest/gtest.h>

#include <sys/syscall.h>
#include <thread>

#include <tbox/util/fs.h>

#include "recorder.h"

namespace tbox {
namespace trace {

void ScopeBoo() {
  RECORD_SCOPE();
  for (int i = 0; i < 10; ++i) {
    RECORD_SCOPE();
    std::this_thread::sleep_for(std::chrono::microseconds(5));
  }
}

void ScopeFoo() {
  RECORD_SCOPE();
  std::this_thread::sleep_for(std::chrono::microseconds(64));
  ScopeBoo();
  std::this_thread::sleep_for(std::chrono::microseconds(10));
}

TEST(Recorder, Base) {
  auto t = std::thread(ScopeFoo);
  ScopeFoo();
  t.join();
}

}
}
