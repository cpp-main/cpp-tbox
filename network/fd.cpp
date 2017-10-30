#include "fd.h"

namespace tbox {
namespace network {

Fd::Fd(int fd) :
    fd_(fd)
{ }

Fd::~Fd()
{
    if (fd_ != -1)
        close(fd_);
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
