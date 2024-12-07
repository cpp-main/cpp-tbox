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
/**
 * 本示例，有名管道的读取
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include <sstream>
#include <cstring>

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/event/loop.h>
#include <tbox/event/fd_event.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;

void PrintUsage(const char *process_name)
{
    cout << "Usage:" << process_name << " epoll|select your_pipefile" << endl;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        PrintUsage(argv[0]);
        return 0;
    }

    LogOutput_Enable();
    Loop* sp_loop = Loop::New(argv[1]);

    const char* pipe_file = argv[2];
    ::unlink(pipe_file);

    int ret = mkfifo(pipe_file, 0666);
    if (ret != 0) {
        LogWarn("mkfifo() ret:%d, errno:%d", ret, errno);
    }
    int fd = open(pipe_file, O_RDONLY|O_NDELAY);
    LogDbg("fd:%d", fd);

    FdEvent* sp_fd_read  = sp_loop->newFdEvent();
    sp_fd_read->initialize(fd, FdEvent::kReadEvent, Event::Mode::kPersist);    //! 可读与挂起事件一直有效
    sp_fd_read->enable();

    sp_fd_read->setCallback(
        [&] (short event) {
            //! 当管道有输入的时候
            if (event & FdEvent::kReadEvent) {
                char input_buff[200];
                int rsize = read(fd, input_buff, sizeof(input_buff));
                if (rsize > 0) {
                    input_buff[rsize - 1] = '\0';
                    LogInfo("read[%d]:%s", rsize, input_buff);
                } else {
                    LogNotice("read 0");
                    sp_fd_read->disable();
                    close(fd);

                    fd = open(pipe_file, O_RDONLY|O_NDELAY);
                    LogDbg("fd:%d", fd);
                    sp_fd_read->initialize(fd, FdEvent::kReadEvent, Event::Mode::kPersist);
                    sp_fd_read->enable();
                }
            }
        }
    );

    sp_loop->runLoop(Loop::Mode::kForever);

    delete sp_fd_read;
    delete sp_loop;

    return 0;
}
