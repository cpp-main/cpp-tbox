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
#include "misc.h"

#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/eventfd.h>

#include <tbox/base/log.h>

namespace tbox {
namespace event {

bool CreateFdPair(int &read_fd, int &write_fd)
{
    int fds[2] = { 0 };
    if (pipe2(fds, O_CLOEXEC | O_NONBLOCK) != 0) {  //!FIXME
        LogErr("pip2() fail, ret:%d", errno);
        return false;
    }

    read_fd = fds[0];
    write_fd = fds[1];
    return true;
}

int CreateEventFd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
        LogErr("eventfd fail, ret:%d", errno);
    return evtfd;
}

}
}
