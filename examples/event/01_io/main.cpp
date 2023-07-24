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
/**
 * 本示例，使用异步FdEvent替代了cout, cin的功能
 */

#include <unistd.h>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <tbox/event/loop.h>
#include <tbox/event/fd_event.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void PrintUsage(const char *process_name)
{
    cout << "Usage:" << process_name << " epoll" << endl;
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    Loop* sp_loop = Loop::New(argv[1]);
    if (sp_loop == nullptr) {
        cout << "fail, exit" << endl;
        return 0;
    }

    FdEvent* sp_fd_read  = sp_loop->newFdEvent();
    FdEvent* sp_fd_write = sp_loop->newFdEvent();

    sp_fd_read->initialize(STDIN_FILENO, FdEvent::kReadEvent, Event::Mode::kPersist);       //! 可读事件一直有效
    sp_fd_write->initialize(STDOUT_FILENO, FdEvent::kWriteEvent, Event::Mode::kOneshot);    //! 可写事件单次有效

    sp_fd_read->enable();   //! 可读是常开的，可写不是

    string send_cache;  //! 发送缓存

    //! 当终端有输入的时候
    sp_fd_read->setCallback(
        [&] (short event) {
            if (event & FdEvent::kReadEvent) {
                char input_buff[200];
                int rsize = read(STDIN_FILENO, input_buff, sizeof(input_buff));
                input_buff[rsize - 1] = '\0';

                stringstream ss;
                ss << "INPUT " << rsize << " : " << input_buff << endl;

                send_cache += ss.str(); //! 放入到send_cache中
                sp_fd_write->enable();  //! 使能发送
            }
        }
    );

    //! 当终端可以输出的时候
    sp_fd_write->setCallback(
        [&] (short event) {
            if (event & FdEvent::kWriteEvent) {
                int wsize = write(STDOUT_FILENO, send_cache.data(), send_cache.size());   //! 尝试全量发送
                if (wsize > 0) {
                    send_cache.erase(0, wsize);  //! 删除已发送的部分
                    if (!send_cache.empty())     //! 如果没有发送完，要继续发
                        sp_fd_write->enable();
                }
            }
        }
    );

    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_fd_read;
    delete sp_fd_write;
    delete sp_loop;

    return 0;
}
