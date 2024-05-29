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
#include "sink.h"

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
  std::string path_prefix = "/tmp/cpp-tbox-test/trace";
  //std::string pid_str = std::to_string(::getpid());
  //std::string time_str = GetTimeStr();

  auto &ts = Sink::GetInstance();
  ts.setPathPrefix(path_prefix);
  ts.enable();
  auto t = std::thread(ScopeFoo);
  ScopeFoo();
  t.join();
  ts.disable();

  util::fs::RemoveDirectory("/tmp/cpp-tbox-test");
}

}
}
