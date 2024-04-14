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
#ifndef TBOX_EVENT_EPOLL_TYPES_H_20230716
#define TBOX_EVENT_EPOLL_TYPES_H_20230716

#include <sys/epoll.h>
#include <vector>

namespace tbox {
namespace event {

class EpollFdEvent;

//! 同一个fd共享的数据
struct EpollFdSharedData {
    int fd = 0;     //!< 文件描述符
    int ref = 0;    //!< 引用计数
    struct epoll_event ev;

    int read_event_num = 0;     //!< 监听可读事件的FdEvent个数
    int write_event_num = 0;    //!< 监听可写事件的FdEvent个数
    int except_event_num = 0;   //!< 监听异常事件的FdEvent个数

    std::vector<EpollFdEvent*> fd_events;
};

}
}

#endif //TBOX_EVENT_EPOLL_TYPES_H_20230716
