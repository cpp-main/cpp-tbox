#include "fd.h"
#include <utility>

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

}
}
