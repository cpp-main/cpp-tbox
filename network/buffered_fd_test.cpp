#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/event/timer_item.h>
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
 * 定时1秒后检查，最后总共应该收到 20 万个 "1234567879\0"
 */

TEST(network_BufferedFd, pipe_test)
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
            cout << "Info: Recv, size(): " << buff.readableSize() << endl;
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
            cout << "Info: All message sent" << endl;
        }
    );
    write_buff_fd->enable();

    //! 创建定时发送的定时器，每20ms一次，执行10次
    int send_times = 0;
    TimerItem* sp_timer_send = sp_loop->newTimerItem();
    sp_timer_send->initialize(std::chrono::milliseconds(20), event::Item::Mode::kPersist);
    sp_timer_send->setCallback(
        [=, &send_times] {
            ++send_times;
            cout << "Info: Send, time: " << send_times << endl;
            for (int i = 0; i < 10000; ++i)
                write_buff_fd->send("123456789", 10);

            if (send_times == 10)
                sp_timer_send->disable();
        }
    );
    sp_timer_send->enable();

    //! 创建退出定时器，定时1秒
    TimerItem* sp_timer_end = sp_loop->newTimerItem();
    sp_timer_end->initialize(std::chrono::seconds(1), event::Item::Mode::kOneshot);
    sp_timer_end->setCallback(
        [=] {
            cout << "Info: Exit Loop" << endl;
            sp_loop->exitLoop();
        }
    );
    sp_timer_end->enable();

    //! 在runLoop()前发送10万个数据
    cout << "Info: Send 100000 package" << endl;
    for (int i = 0; i < 100000; ++i)
        write_buff_fd->send("123456789", 10);

    cout << "Info: Start Loop" << endl;
    sp_loop->runLoop();
    cout << "Info: End Loop" << endl;

    EXPECT_EQ(recv_count, 200000);  //! 检查是否共收到20万的数据包

    delete sp_timer_end;
    delete sp_timer_send;
    delete write_buff_fd;
    delete read_buff_fd;
    delete sp_loop;
}
