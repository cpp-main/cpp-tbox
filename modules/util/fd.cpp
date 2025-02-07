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
#include "fd.h"

#include <utility>
#include <fcntl.h>
#include <errno.h>

#include <tbox/base/defines.h>
#include <tbox/base/log.h>
#include <tbox/base/assert.h>

namespace tbox {
namespace util {

Fd::Fd() { }

Fd::Fd(int fd) {
    detail_ = new Detail;
    detail_->fd = fd;
}

Fd::Fd(int fd, const CloseFunc &close_func) {
    detail_ = new Detail;
    detail_->fd = fd;
    detail_->close_func = close_func;
}

Fd::~Fd()
{
    if (detail_ == nullptr)
        return;

    TBOX_ASSERT(detail_->ref_count > 0);

    --detail_->ref_count;
    if (detail_->ref_count == 0) {
        if (detail_->fd >= 0) {
            if (detail_->close_func)
                detail_->close_func(detail_->fd);
            else
                ::close(detail_->fd);
        }
        delete detail_;
    }
}

Fd::Fd(const Fd& other)
{
    if (other.detail_ != nullptr) {
        ++other.detail_->ref_count;
        detail_ = other.detail_;
    }
}

Fd& Fd::operator = (const Fd& other)
{
    if (this != &other) {
        reset();
        if (other.detail_ != nullptr) {
            ++other.detail_->ref_count;
            detail_ = other.detail_;
        }
    }
    return *this;
}

void Fd::swap(Fd &other)
{
    std::swap(detail_, other.detail_);
}

IMPL_MOVE_RESET_FUNC(Fd)

void Fd::close()
{
    if (detail_ != nullptr && detail_->fd >= 0) {
        if (detail_->close_func) {
            detail_->close_func(detail_->fd);
            detail_->close_func = nullptr;
        } else
            ::close(detail_->fd);
        detail_->fd = -1;
    }
}

ssize_t Fd::read(void *ptr, size_t size) const
{
    if (detail_ == nullptr)
        return -1;

    return ::read(detail_->fd, ptr, size);
}

ssize_t Fd::readv(const struct iovec *iov, int iovcnt) const
{
    if (detail_ == nullptr)
        return -1;

    return ::readv(detail_->fd, iov, iovcnt);
}

ssize_t Fd::write(const void *ptr, size_t size) const
{
    if (detail_ == nullptr)
        return -1;

    return ::write(detail_->fd, ptr, size);
}

ssize_t Fd::writev(const struct iovec *iov, int iovcnt) const
{
    if (detail_ == nullptr)
        return -1;

    return ::writev(detail_->fd, iov, iovcnt);
}

void Fd::setNonBlock(bool enable) const
{
    if (detail_ == nullptr)
        return;

#ifdef O_NONBLOCK
    int old_flags = fcntl(detail_->fd, F_GETFL, 0);
    int new_flags = old_flags;

    if (enable)
        new_flags |= O_NONBLOCK;
    else
        new_flags &= ~O_NONBLOCK;

    if (new_flags != old_flags) {
        int ret = fcntl(detail_->fd, F_SETFL, new_flags);
        if (ret == -1)
            LogErr("fcntl error, errno:%d", errno);
    }
#else
#error No way found to set non-blocking mode for fds.
#endif
}

bool Fd::isNonBlock() const
{
    if (detail_ == nullptr)
        return false;

#ifdef O_NONBLOCK
    int flags = fcntl(detail_->fd, F_GETFL, 0);
    return (flags & O_NONBLOCK) != 0;
#else
#error No way found to set non-blocking mode for fds.
#endif
}

void Fd::setCloseOnExec() const
{
    if (detail_ == nullptr)
        return;

#ifdef FD_CLOEXEC
    int old_flags = fcntl(detail_->fd, F_GETFD, 0);
    int new_flags = old_flags | FD_CLOEXEC;
    if (new_flags != old_flags) {
        int ret = fcntl(detail_->fd, F_SETFL, new_flags);
        if (ret == -1)
            LogErr("fcntl error, errno:%d", errno);
    }
#else
    UNUSED(fd);
#endif
}

Fd Fd::Open(const char *filename, int flags)
{
    int fd = ::open(filename, flags);
    if (fd < 0) {
        LogErr("open file:%s flags:%x fail, errno:%d", filename, flags, errno);
        return Fd();
    }

    return Fd(fd);
}

}
}
