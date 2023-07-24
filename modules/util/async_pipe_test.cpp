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
#include "async_pipe.h"
#include <gtest/gtest.h>
#include <vector>
#include <thread>

using namespace tbox::util;
using namespace std;

void TestByConfig(AsyncPipe::Config cfg)
{
    vector<uint8_t> out_data;

    AsyncPipe ap;
    EXPECT_TRUE(ap.initialize(cfg));
    ap.setCallback(
        [&] (const void *ptr, size_t size) {
            const uint8_t *p = static_cast<const uint8_t*>(ptr);
            for (size_t i = 0; i < size; ++i)
                out_data.push_back(p[i]);
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    );


    for (size_t i = 0; i < 256; ++i) {
        uint8_t v = i;
        ap.append(&v, 1);
    }
    ap.cleanup();

    EXPECT_EQ(out_data.size(), 256);
    for (size_t i = 0; i < 256; ++i) {
        EXPECT_EQ(out_data[i], i);
    }
}

TEST(AsyncPipe, Size1Num1)
{
    AsyncPipe::Config cfg;
    cfg.buff_size = 1;
    cfg.buff_min_num  = 1;
    cfg.buff_max_num  = 1;
    cfg.interval = 10;

    TestByConfig(cfg);
}

TEST(AsyncPipe, Size1Num2)
{
    AsyncPipe::Config cfg;
    cfg.buff_size = 1;
    cfg.buff_min_num  = 2;
    cfg.buff_max_num  = 2;
    cfg.interval = 10;

    TestByConfig(cfg);
}

TEST(AsyncPipe, Size1MinNum2MaxNum10)
{
    AsyncPipe::Config cfg;
    cfg.buff_size = 1;
    cfg.buff_min_num  = 2;
    cfg.buff_max_num  = 10;
    cfg.interval = 10;

    TestByConfig(cfg);
}

TEST(AsyncPipe, Size2Num1)
{
    AsyncPipe::Config cfg;
    cfg.buff_size = 2;
    cfg.buff_min_num  = 1;
    cfg.buff_max_num  = 1;
    cfg.interval = 10;

    TestByConfig(cfg);
}

TEST(AsyncPipe, Size50Num1)
{
    AsyncPipe::Config cfg;
    cfg.buff_size = 50;
    cfg.buff_min_num  = 1;
    cfg.buff_max_num  = 1;
    cfg.interval = 10;

    TestByConfig(cfg);
}

TEST(AsyncPipe, Size50Num2)
{
    AsyncPipe::Config cfg;
    cfg.buff_size = 50;
    cfg.buff_min_num  = 2;
    cfg.buff_max_num  = 2;
    cfg.interval = 10;

    TestByConfig(cfg);
}

TEST(AsyncPipe, Size50Num3)
{
    AsyncPipe::Config cfg;
    cfg.buff_size = 50;
    cfg.buff_min_num  = 3;
    cfg.buff_max_num  = 3;
    cfg.interval = 10;

    TestByConfig(cfg);
}

TEST(AsyncPipe, Size500Num1)
{
    AsyncPipe::Config cfg;
    cfg.buff_size = 500;
    cfg.buff_min_num  = 1;
    cfg.buff_max_num  = 1;
    cfg.interval = 10;

    TestByConfig(cfg);
}

/**
 * 测试周期性同步
 *
 * 往里写一个比缓冲大小小的数据，观察其是否在指定间隔内进行了同步
 */
TEST(AsyncPipe, PeriodSync)
{
    AsyncPipe::Config cfg;
    cfg.buff_size = 500;
    cfg.buff_min_num  = 1;
    cfg.buff_max_num  = 1;
    cfg.interval = 10;

    vector<uint8_t> out_data;

    AsyncPipe ap;
    EXPECT_TRUE(ap.initialize(cfg));
    ap.setCallback(
        [&] (const void *ptr, size_t size) {
            const uint8_t *p = static_cast<const uint8_t*>(ptr);
            for (size_t i = 0; i < size; ++i)
                out_data.push_back(p[i]);
        }
    );

    uint8_t dummy = 12;
    ap.append(&dummy, 1);
    EXPECT_EQ(out_data.size(), 0);

    this_thread::sleep_for(chrono::milliseconds(15));
    ASSERT_EQ(out_data.size(), 1);
    EXPECT_EQ(out_data[0], 12);
    ap.cleanup();
}

TEST(AsyncPipe, ForgetCleanup)
{
    AsyncPipe::Config cfg;
    cfg.buff_size = 500;
    cfg.buff_min_num  = 3;
    cfg.buff_max_num  = 3;
    cfg.interval = 10;

    vector<uint8_t> out_data;

    AsyncPipe ap;
    EXPECT_TRUE(ap.initialize(cfg));
    ap.setCallback(
        [&] (const void *ptr, size_t size) {
            const uint8_t *p = static_cast<const uint8_t*>(ptr);
            for (size_t i = 0; i < size; ++i)
                out_data.push_back(p[i]);
        }
    );

    uint8_t dummy = 12;
    ap.append(&dummy, 1);
    EXPECT_EQ(out_data.size(), 0);

    this_thread::sleep_for(chrono::milliseconds(15));
    ASSERT_EQ(out_data.size(), 1);
    EXPECT_EQ(out_data[0], 12);
    ap.cleanup();
}
