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
#include "recorder.h"

namespace tbox {
namespace trace {

TEST(Recorder, Scope) {
  RECORD_SCOPE();
  if (true) {
    RECORD_SCOPE();
  }
}

TEST(Recorder, Event) {
  RECORD_EVENT();
  RECORD_EVENT();
}

TEST(Recorder, Named) {
  RECORD_DEFINE(a);
  RECORD_DEFINE(b);

  RECORD_START(a);
  RECORD_START(b);

  RECORD_STOP(a);
  RECORD_STOP(b);
}

}
}
