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
#include <sys/select.h>
#include <fcntl.h>

#include <cstring>
#include <algorithm>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>

#include "loop.h"
#include "fd_event.h"

namespace tbox {
namespace event {

SelectLoop::~SelectLoop()
{
    cleanupDeferredTasks();
}

void SelectLoop::runLoop(Mode mode)
{
    RECORD_EVENT();

    runThisBeforeLoop();

    keep_running_ = (mode == Loop::Mode::kForever);
    do {
        fd_set read_set, write_set, except_set;
        int nfds = fillFdSets(read_set, write_set, except_set);

        struct timeval tv, *p_tv = nullptr;
        auto wait_ms = getWaitTime();
        if (wait_ms != -1) {
            tv.tv_sec = wait_ms / 1000;
            tv.tv_usec = wait_ms % 1000;
            p_tv = &tv;
        }

        int select_ret = ::select(nfds, &read_set, &write_set, &except_set, p_tv);

        RECORD_SCOPE();
        beginLoopProcess();

        handleExpiredTimers();

        if (select_ret > 0) {
            for (int fd = 0; fd < nfds; ++fd) {
                bool is_readable = FD_ISSET(fd, &read_set);
                bool is_writable = FD_ISSET(fd, &write_set);
                bool is_except   = FD_ISSET(fd, &except_set);

                if (is_readable || is_writable || is_except) {
                    auto *data = fd_data_map_.at(fd);
                    SelectFdEvent::OnEventCallback(is_readable, is_writable, is_except, data);
                }
            }
        } else if (select_ret == -1) {
            if (errno == EBADF) {
                removeInvalidFds();

            } else if (errno != EINTR) {
                LogErrno(errno, "select error");
                break;

            } else {
                LogNotice("select errno:%d(%s)", errno, strerror(errno));
            }
        }

        //handleRunInLoopFunc();
        handleNextFunc();

        endLoopProcess();

    } while (keep_running_);

    runThisAfterLoop();

    RECORD_EVENT();
}

int SelectLoop::fillFdSets(fd_set &read_set, fd_set &write_set, fd_set &except_set)
{
    int max_fd = -1;

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_ZERO(&except_set);

    for (auto item : fd_data_map_) {
        auto fd = item.first;

        if (fd < 0)
            continue;

        bool is_this_fd_used = false;
        auto data = item.second;

        if (data->read_event_num > 0) {
            FD_SET(fd, &read_set);
            is_this_fd_used = true;
        }

        if (data->write_event_num > 0) {
            FD_SET(fd, &write_set);
            is_this_fd_used = true;
        }

        if (data->except_event_num > 0) {
            FD_SET(fd, &except_set);
            is_this_fd_used = true;
        }

        if (is_this_fd_used && max_fd < fd)
            max_fd = fd;
    }

    return max_fd + 1;
}

bool IsFdValid(int fd)
{
    if (fd < 0)
        return false;

    return ::fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

void SelectLoop::removeInvalidFds()
{
    for (auto item : fd_data_map_) {
        auto fd = item.first;
        if (!IsFdValid(fd)) {
            LogWarn("fd:%d is invalid", fd);
            SelectFdSharedData *data = item.second;
            for (auto event : data->fd_events) {
                event->disable();
            }
        }
    }
}

SelectFdSharedData* SelectLoop::refFdSharedData(int fd)
{
    SelectFdSharedData *fd_shared_data = nullptr;

    auto it = fd_data_map_.find(fd);
    if (it != fd_data_map_.end())
        fd_shared_data = it->second;

    if (fd_shared_data == nullptr) {
        fd_shared_data = fd_shared_data_pool_.alloc();
        TBOX_ASSERT(fd_shared_data != nullptr);
        fd_data_map_.insert(std::make_pair(fd, fd_shared_data));
    }

    ++fd_shared_data->ref;
    return fd_shared_data;
}

void SelectLoop::unrefFdSharedData(int fd)
{
    auto it = fd_data_map_.find(fd);
    if (it != fd_data_map_.end()) {
        auto fd_shared_data = it->second;
        --fd_shared_data->ref;
        if (fd_shared_data->ref == 0) {
            fd_data_map_.erase(fd);
            fd_shared_data_pool_.free(fd_shared_data);
        }
    }
}

FdEvent* SelectLoop::newFdEvent(const std::string &what)
{
    return new SelectFdEvent(this, what);
}

}
}
