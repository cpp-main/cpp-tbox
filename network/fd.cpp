#include "fd.h"

#include <utility>
#include <fcntl.h>
#include <errno.h>

#include <tbox/base/log.h>

namespace tbox {
namespace network {

Fd::Fd() { }
Fd::Fd(int fd) : fd_(fd) { }

Fd::~Fd()
{
    CHECK_CLOSE_FD(fd_);
}

Fd::Fd(Fd&& other)
{
    fd_ = other.fd_;
    other.fd_ = -1;
}

Fd& Fd::operator = (Fd&& other)
{
    if (this != &other) {
        reset();
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

void Fd::swap(Fd &other)
{
    std::swap(fd_, other.fd_);
}

void Fd::reset()
{
    Fd tmp;
    swap(tmp);
}

ssize_t Fd::read(void *ptr, size_t size) const
{
    return ::read(fd_, ptr, size);
}

ssize_t Fd::readv(const struct iovec *iov, int iovcnt) const
{
    return ::readv(fd_, iov, iovcnt);
}

ssize_t Fd::write(const void *ptr, size_t size) const
{
    return ::write(fd_, ptr, size);
}

ssize_t Fd::writev(const struct iovec *iov, int iovcnt) const
{
    return ::writev(fd_, iov, iovcnt);
}

void Fd::setNonBlock(bool enable) const
{
#ifdef O_NONBLOCK
    int old_flags = fcntl(fd_, F_GETFL, 0);
    int new_flags = old_flags;

    if (enable)
        new_flags |= O_NONBLOCK;
    else
        new_flags &= ~O_NONBLOCK;

    if (new_flags != old_flags) {
        int ret = fcntl(fd_, F_SETFL, new_flags);
        if (ret == -1)
            LogErr("fcntl error, errno:%d", errno);
    }
#else
#error No way found to set non-blocking mode for fds.
#endif
}

bool Fd::isNonBlock() const
{
#ifdef O_NONBLOCK
    int flags = fcntl(fd_, F_GETFL, 0);
    return (flags & O_NONBLOCK) != 0;
#else
#error No way found to set non-blocking mode for fds.
#endif
}

void Fd::setCloseOnExec() const
{
#ifdef FD_CLOEXEC
    int old_flags = fcntl(fd_, F_GETFD, 0);
    int new_flags = old_flags | FD_CLOEXEC;
    if (new_flags != old_flags) {
        int ret = fcntl(fd_, F_SETFL, new_flags);
        if (ret == -1)
            LogErr("fcntl error, errno:%d", errno);
    }
#else
    UNUSED(fd);
#endif
}

}
}
