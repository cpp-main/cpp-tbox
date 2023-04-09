#include <sys/epoll.h>
#include <unistd.h>

#include <cstdint>

#include <vector>
#include <algorithm>

#include "loop.h"
#include "fd_event.h"

#include <tbox/base/log.h>
#include <tbox/base/defines.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace event {

EpollLoop::EpollLoop() :
    epoll_fd_(epoll_create1(EPOLL_CLOEXEC))
{
    TBOX_ASSERT(epoll_fd_ >= 0);
}

EpollLoop::~EpollLoop()
{
    cleanupDeferredTasks();

    CHECK_CLOSE_RESET_FD(epoll_fd_);
}

void EpollLoop::runLoop(Mode mode)
{
    if (epoll_fd_ < 0)
        return;

    std::vector<struct epoll_event> events;
    /*
     * Why not events.reserve()?
     * The reserve() method only allocates memory, but leaves it uninitialized,
     * it only affects capacity(), but size() will be unchanged.
     * The standard only guarantees that std::vector::data returns a pointer and [data(), data() + size()] is a valid range,
     * the capacity is not concerned. So we need use resize and ensure the [data(), data() + size()] is a valid range whitch used by epoll_wait.
     */
    events.resize(max_loop_entries_);

    runThisBeforeLoop();

    keep_running_ = (mode == Loop::Mode::kForever);
    do {
        startWaitEvents();

        int fds = epoll_wait(epoll_fd_, events.data(), events.size(), getWaitTime());

        startHandleEvents();

        handleExpiredTimers();

        for (int i = 0; i < fds; ++i) {
            epoll_event &ev = events.at(i);
            EpollFdEvent::OnEventCallback(ev.data.fd, ev.events, ev.data.ptr);
        }

        handleNextFunc();

        /// If the receiver array size is full, increase its size with 1.5 times.
        if (UNLIKELY(fds >= max_loop_entries_)) {
            max_loop_entries_ = (max_loop_entries_ + max_loop_entries_ / 2);
            events.resize(max_loop_entries_);
        }

    } while (keep_running_);

    runThisAfterLoop();
}

void EpollLoop::addFdSharedData(int fd, EpollFdSharedData *fd_event)
{
    fd_data_map_.insert(std::make_pair(fd, fd_event));
}

void EpollLoop::removeFdSharedData(int fd)
{
    fd_data_map_.erase(fd);
}

EpollFdSharedData* EpollLoop::queryFdSharedData(int fd) const
{
    auto it = fd_data_map_.find(fd);
    if (it != fd_data_map_.end())
        return it->second;
    return nullptr;
}


FdEvent* EpollLoop::newFdEvent()
{
    return new EpollFdEvent(this);
}

}
}
