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
#include "loop_thread.h"

#include <gtest/gtest.h>

namespace tbox {
namespace eventx {
namespace {

TEST(LoopThread, runNow) {
    bool tag = false;
    {
        LoopThread lp;
        lp.loop()->runInLoop([&] { tag = true; });
    }
    EXPECT_TRUE(tag);
}

TEST(LoopThread, runLater) {
    bool tag = false;
    LoopThread lp(false);
    lp.loop()->runInLoop([&] { tag = true; });
    lp.start();
    lp.stop();
    EXPECT_TRUE(tag);
}

}
}
}
