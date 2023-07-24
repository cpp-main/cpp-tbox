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
#include <gtest/gtest.h>
#include "stat.h"
#include <ostream>

namespace tbox {
namespace event {

using namespace std;

TEST(Stat, Stream)
{
    const char *target_str = R"(stat_time: 10000 us
time_cost: 60 us
max_time_cost: 50 us
event_count: 2
avg_cost: 30 us
cpu: 0.6 %
)";

    Stat stat;
    stat.event_count = 2;
    stat.peak_cost_us = 50;
    stat.time_cost_us = 60;
    stat.stat_time_us = 10000;

    ostringstream oss;
    oss << stat;

    EXPECT_EQ(oss.str(), target_str);
}

}
}
