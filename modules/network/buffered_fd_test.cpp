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
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/network/buffered_fd.h>

#include <unistd.h>
#include <iostream>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;

/**
 * 测试方法：
 * 在 runLoop() 之前，一次性塞入 10 万个 "123456789\0"。
 * 并注册一个定时器，每20ms发送 1 万个 "123456789\0"，共分10次进行。
 * 定时2秒后检查，最后总共应该收到 20 万个 "1234567879\0"
 */

TEST(BufferedFd, pipe_test)
{
    Loop* sp_loop = Loop::New();
    ASSERT_TRUE(sp_loop);

    int fds[2] = { 0 };
    ASSERT_EQ(pipe(fds), 0);

    int recv_count = 0;

    //! 创建接收的BufferedFd
    BufferedFd *read_buff_fd = new BufferedFd(sp_loop);
    read_buff_fd->initialize(fds[0]);
    read_buff_fd->setReceiveCallback(
        [&recv_count] (Buffer &buff){
            //cout << "Info: Recv, size(): " << buff.readableSize() << endl;
            while (buff.readableSize() >= 10) {
                char read_data[10];
                buff.fetch(read_data, 10);
                ASSERT_STREQ(read_data, "123456789");   //! 检查每个数据包收到的内容是否正确
                ++recv_count;
            }
        }
    , 10);
    read_buff_fd->enable();

    //! 创建发送的BufferedFd
    BufferedFd *write_buff_fd = new BufferedFd(sp_loop);
    write_buff_fd->initialize(fds[1]);
    write_buff_fd->setSendCompleteCallback(
        [] {
            //cout << "Info: All message sent" << endl;
        }
    );
    write_buff_fd->enable();

    //! 创建定时发送的定时器，每20ms一次，执行10次
    int send_times = 0;
    TimerEvent* sp_timer_send = sp_loop->newTimerEvent();
    sp_timer_send->initialize(std::chrono::milliseconds(20), event::Event::Mode::kPersist);
    sp_timer_send->setCallback(
        [=, &send_times] {
            ++send_times;
            //cout << "Info: Send, time: " << send_times << endl;
            for (int i = 0; i < 10000; ++i)
                write_buff_fd->send("123456789", 10);

            if (send_times == 10)
                sp_timer_send->disable();
        }
    );
    sp_timer_send->enable();

    //! 在runLoop()前发送10万个数据
    //cout << "Info: Send 100000 package" << endl;
    for (int i = 0; i < 100000; ++i)
        write_buff_fd->send("123456789", 10);

    //cout << "Info: Start Loop" << endl;
    sp_loop->exitLoop(std::chrono::seconds(2));
    sp_loop->runLoop();
    //cout << "Info: End Loop" << endl;

    EXPECT_EQ(recv_count, 200000);  //! 检查是否共收到20万的数据包

    CHECK_CLOSE_RESET_FD(fds[0]);
    CHECK_CLOSE_RESET_FD(fds[1]);

    delete sp_timer_send;
    delete write_buff_fd;
    delete read_buff_fd;
    delete sp_loop;
}

//! 测试在发送小数据完成之后，有没有回调setSendCompleteCallback()对应的函数
TEST(BufferedFd, sendComplete_LittleData)
{
    Loop* sp_loop = Loop::New();
    ASSERT_TRUE(sp_loop);

    int fds[2] = { 0 };
    ASSERT_EQ(pipe(fds), 0);

    int recv_count = 0;

    //! 创建接收的BufferedFd
    BufferedFd *read_buff_fd = new BufferedFd(sp_loop);
    read_buff_fd->initialize(fds[0]);
    read_buff_fd->setReceiveCallback(
        [&] (Buffer &buff) {
            recv_count += buff.readableSize();
            buff.hasReadAll();
        }, 0
    );
    read_buff_fd->enable();

    bool is_send_completed = false;
    //! 创建发送的BufferedFd
    BufferedFd *write_buff_fd = new BufferedFd(sp_loop);
    write_buff_fd->initialize(fds[1]);
    write_buff_fd->setSendCompleteCallback(
        [&] {
            write_buff_fd->disable();
            CHECK_CLOSE_RESET_FD(fds[1]);
            is_send_completed = true;
        }
    );
    write_buff_fd->enable();

    //! 发送10B数据
    std::vector<uint8_t> send_data(10, 0);
    write_buff_fd->send(send_data.data(), send_data.size());

    sp_loop->exitLoop(std::chrono::milliseconds(10));
    sp_loop->runLoop();

    EXPECT_EQ(recv_count, send_data.size());
    EXPECT_TRUE(is_send_completed);

    CHECK_CLOSE_RESET_FD(fds[0]);
    delete read_buff_fd;
    delete write_buff_fd;
    delete sp_loop;
}

//! 测试在发送大数据完成之后，有没有回调setSendCompleteCallback()对应的函数
TEST(BufferedFd, sendComplete_HugeData)
{
    Loop* sp_loop = Loop::New();
    ASSERT_TRUE(sp_loop);

    int fds[2] = { 0 };
    ASSERT_EQ(pipe(fds), 0);

    int recv_count = 0;

    //! 创建接收的BufferedFd
    BufferedFd *read_buff_fd = new BufferedFd(sp_loop);
    read_buff_fd->initialize(fds[0]);
    read_buff_fd->setReceiveCallback(
        [&] (Buffer &buff) {
            recv_count += buff.readableSize();
            buff.hasReadAll();
        }, 0
    );
    read_buff_fd->enable();

    bool is_send_completed = false;
    //! 创建发送的BufferedFd
    BufferedFd *write_buff_fd = new BufferedFd(sp_loop);
    write_buff_fd->initialize(fds[1]);
    write_buff_fd->setSendCompleteCallback(
        [&] {
            write_buff_fd->disable();
            CHECK_CLOSE_RESET_FD(fds[1]);
            is_send_completed = true;
        }
    );
    write_buff_fd->enable();

    //! 一次性发送128M数据
    std::vector<uint8_t> send_data(128ul << 20, 0);
    write_buff_fd->send(send_data.data(), send_data.size());

    sp_loop->exitLoop(std::chrono::seconds(2));
    sp_loop->runLoop();

    EXPECT_EQ(recv_count, send_data.size());
    EXPECT_TRUE(is_send_completed);

    CHECK_CLOSE_RESET_FD(fds[0]);
    delete read_buff_fd;
    delete write_buff_fd;
    delete sp_loop;
}
