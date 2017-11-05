#include <unistd.h>
#include <iostream>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <gtest/gtest.h>
#include "uart.h"

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;

/**
 * 对串口进行环回测试
 *
 * 本测试需要采用USB转串口模块，其它系统中的设备文件为：/dev/ttyUSB0
 * 并将该串口模块的 TXD 与 RXD 进行短接
 * 如果打开设备文件失败，请检查是否存在 /dev/ttyUSB0 文件，并使用 root 权限执行测试用例
 */
TEST(network_uart, echo) {
    Loop* sp_loop = Loop::New();
    SetScopeExitAction([=]{ delete sp_loop; });

    Uart uart(sp_loop);

    ASSERT_TRUE(uart.initialize("/dev/ttyUSB0"))
        << "NOTICE:" << endl
        << "1) check whether /dev/ttyUSB0 exist." << endl
        << "2) run test as root.";
    ASSERT_TRUE(uart.setMode("115200 8n1"))
        << "NOTICE:" << endl
        << "Is this UART support 115200 8N1 ?";
    uart.enable();

    int recv_count = 0;
    uart.setReceiveCallback(
        [&] (Buffer &buff) {
            //cout << "Info: Recv, size: " << buff.readableSize() << endl;
            while (buff.readableSize() >= 10) {
                char read_data[10];
                buff.fetch(read_data, 10);
                ASSERT_STREQ(read_data, "123456789");   //! 检查每个数据包收到的内容是否正确
                ++recv_count;
            }
        }, 0);

    //! 创建数据发送定时器
    auto sp_timer_send = sp_loop->newTimerEvent();
    SetScopeExitAction([=]{ delete sp_timer_send; });
    sp_timer_send->initialize(chrono::milliseconds(20), Event::Mode::kPersist);
    sp_timer_send->enable();
    int send_times = 0;
    sp_timer_send->setCallback(
        [&] () {
            ++send_times;
            //cout << "Info: Send, time: " << send_times << endl;
            for (int i = 0; i < 100; ++i)
                uart.send("123456789", 10);

            if (send_times == 10)
                sp_timer_send->disable();
        }
    );

    //! 创建停止定时器
    auto sp_timer_end = sp_loop->newTimerEvent();
    SetScopeExitAction([=]{ delete sp_timer_end; });
    sp_timer_end->initialize(chrono::seconds(3), Event::Mode::kOneshot);
    sp_timer_end->enable();
    sp_timer_end->setCallback(
        [=] () {
            sp_loop->exitLoop();
        }
    );

    for (int i = 0; i < 1000; ++i)
        uart.send("123456789", 10);

    cout << "Info: Loop begin" << endl;
    sp_loop->runLoop();
    cout << "Info: Loop end" << endl;

    EXPECT_EQ(recv_count, 2000)
        << "NOTICE: please connect TXD and RXD together.";  //! 检查总共收到的数据包总数
}

