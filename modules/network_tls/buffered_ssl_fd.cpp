/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S E  /  \/ \
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
#include "buffered_ssl_fd.h"

#include <cstring>
#include <openssl/err.h>
#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/wrapped_recorder.h>
#include <tbox/event/loop.h>
#include <tbox/event/fd_event.h>

namespace tbox {
namespace network {

BufferedSslFd::BufferedSslFd(event::Loop *wp_loop) :
    BufferedFd(wp_loop)
{ }

BufferedSslFd::~BufferedSslFd()
{
    if (ssl_ != nullptr) {
        //! 尝试优雅关闭 SSL 连接
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
        ssl_ = nullptr;
    }
}

bool BufferedSslFd::initialize(Fd fd, SSL *ssl, short events)
{
    if (ssl == nullptr) {
        LogWarn("ssl is null");
        return false;
    }

    ssl_ = ssl;

    //! 设置 SSL 模式：允许写缓冲区移动（因为我们的 send buffer 在发送过程中可能被修改）
    SSL_set_mode(ssl_, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);

    //! 调用父类 initialize
    return BufferedFd::initialize(fd, events);
}

ssize_t BufferedSslFd::doReadv(const struct iovec *iov, int iovcnt)
{
    //! SSL_read 只能向单一连续 buffer 写入数据，无法像 readv() 那样 scatter-gather
    //! 实现：逐个 iov 调用 doRead()，将 SSL 解密数据依次填入各 iov 缓冲区
    //! 已填满的 iov 不再参与后续读取，确保数据不会溢出

    ssize_t total_read = 0;
    for (int i = 0; i < iovcnt; ++i) {
        if (iov[i].iov_len == 0)
            continue;

        ssize_t rsize = doRead(iov[i].iov_base, iov[i].iov_len);
        if (rsize > 0) {
            total_read += rsize;
            //! 如果本次读取未填满当前 iov，说明 SSL 内部缓冲已暂无更多数据
            //! 后续 iov 也无法再填充，直接返回已读总量
            if (static_cast<size_t>(rsize) < iov[i].iov_len)
                return total_read;
        } else if (rsize == 0) {
            //! 对端关闭连接（close_notify），立即返回
            //! 若已有部分数据读入前面 iov，total_read > 0，由上层判断
            //! 若无数据读入，total_read == 0，上层会走 read_zero_cb_ 逻辑
            return (total_read > 0) ? total_read : 0;
        } else {
            //! 读取出错（EAGAIN 或其他错误）
            //! 若已有部分数据读入前面 iov，total_read > 0，优先返回已读数据
            //! errno 已由 doRead 设置（EAGAIN 等），上层据此判断
            return (total_read > 0) ? total_read : -1;
        }
    }

    //! 所有 iov 都被填满，但 SSL 内部缓冲可能还有 pending 数据
    //! 上层 onReadCallback 的 while 循环会继续调用 doReadv 来提取
    return total_read;
}

ssize_t BufferedSslFd::doRead(void *buffer, size_t size)
{
    RECORD_SCOPE();

    if (ssl_ == nullptr)
        return -1;

    ERR_clear_error();
    ssize_t rsize = SSL_read(ssl_, buffer, size);

    if (rsize > 0) {
        //! 读取成功，清除 renegotiation 标记
        ssl_read_wants_write_ = false;
        ssl_write_wants_read_ = false;
        return rsize;
    }

    int ssl_error = SSL_get_error(ssl_, rsize);

    if (ssl_error == SSL_ERROR_ZERO_RETURN) {
        //! 对端关闭连接（close_notify）
        errno = 0;  //! 这不是错误，类似 read 返回 0
        return 0;
    }

    if (ssl_error == SSL_ERROR_WANT_READ) {
        //! 需要更多网络数据才能完成 SSL_read，等下次读事件触发即可
        //! 返回 -1 并设置 errno = EAGAIN，让上层 BufferedFd::onReadCallback 知道暂时没数据
        errno = EAGAIN;
        return -1;
    }

    if (ssl_error == SSL_ERROR_WANT_WRITE) {
        //! renegotiation：SSL_read 需要写入数据
        //! 需要临时启用写事件
        handleSslRenegotiation(ssl_error, true);
        errno = EAGAIN;
        return -1;
    }

    if (ssl_error == SSL_ERROR_SYSCALL) {
        //! 系统调用错误
        if (errno == 0) {
            //! EOF：对端关闭了连接（未发送 close_notify）
            return 0;
        }
        //! 其他系统错误，errno 已由系统设置
        return -1;
    }

    if (ssl_error == SSL_ERROR_SSL) {
        //! SSL 协议错误
        //! 当 errno == 0 时，通常是对端非正常关闭连接（未发送 close_notify）
        //! 这在实践中很常见，应该视为正常断开，而不是错误
        if (errno == 0) {
            return 0;
        }
        //! 其他 SSL 协议错误
        LogWarn("SSL_read error: SSL_ERROR_SSL, errno:%d", errno);
        errno = ECONNRESET;
        return -1;
    }

    //! 其他未知 SSL 错误
    LogWarn("SSL_read error: %d, errno:%d", ssl_error, errno);
    errno = ECONNRESET;  //! 将 SSL 错误映射为连接错误
    return -1;
}

ssize_t BufferedSslFd::doWrite(const void *data, size_t size)
{
    RECORD_SCOPE();

    if (ssl_ == nullptr)
        return -1;

    ERR_clear_error();
    ssize_t wsize = SSL_write(ssl_, data, size);

    if (wsize > 0) {
        //! 写入成功，清除 renegotiation 标记
        ssl_read_wants_write_ = false;
        ssl_write_wants_read_ = false;
        return wsize;
    }

    int ssl_error = SSL_get_error(ssl_, wsize);

    if (ssl_error == SSL_ERROR_WANT_WRITE) {
        //! 需要等 fd 可写才能继续 SSL_write
        //! 返回 0 表示"没写成功但不是错误"，让 BufferedFd 保持数据在 send buffer
        //! 注意：不能返回 -1 因为 BufferedFd 的 send() 会把 -1 当成错误丢弃数据
        errno = EAGAIN;
        //! 返回 0 让 BufferedFd::send() 认为"未发送"，数据留在 send buffer
        return 0;
    }

    if (ssl_error == SSL_ERROR_WANT_READ) {
        //! renegotiation：SSL_write 需要读取数据
        handleSslRenegotiation(ssl_error, false);
        errno = EAGAIN;
        return 0;
    }

    if (ssl_error == SSL_ERROR_ZERO_RETURN) {
        //! 对端发送了 close_notify
        errno = EPIPE;
        return -1;
    }

    if (ssl_error == SSL_ERROR_SYSCALL) {
        if (errno == 0) {
            errno = EPIPE;
            return -1;
        }
        return -1;
    }

    //! 其他 SSL 错误
    LogWarn("SSL_write error: %d", ssl_error);
    errno = ECONNRESET;
    return -1;
}

void BufferedSslFd::handleSslRenegotiation(int ssl_error, bool is_read_op)
{
    if (is_read_op && ssl_error == SSL_ERROR_WANT_WRITE) {
        //! SSL_read 需要 fd 可写（renegotiation）
        ssl_read_wants_write_ = true;
        //! 临时启用写事件，即使 send buffer 为空
        //! 注意：onWriteCallback 在 send buffer 为空时会检查此标记，
        //! 如果为 true 则不清除标记、不关闭写事件，而是继续驱动 SSL_read
    } else if (!is_read_op && ssl_error == SSL_ERROR_WANT_READ) {
        //! SSL_write 需要 fd 可读（renegotiation）
        ssl_write_wants_read_ = true;
        //! 读事件已经持续启用，无需额外操作
    }
}

}
}
