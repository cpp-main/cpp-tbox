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
    int ref = 0;    //! 引用计数
    struct epoll_event ev;
    std::vector<EpollFdEvent*> read_events;
    std::vector<EpollFdEvent*> write_events;
    std::vector<EpollFdEvent*> exception_events;
};

}
}

#endif //TBOX_EVENT_EPOLL_TYPES_H_20230716
