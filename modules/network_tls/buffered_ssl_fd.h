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
#ifndef TBOX_NETWORK_BUFFERED_SSL_FD_H_20260616
#define TBOX_NETWORK_BUFFERED_SSL_FD_H_20260616

#include <openssl/ssl.h>

#include <tbox/network/buffered_fd.h>

namespace tbox {
namespace network {

//! 基于 SSL 的 BufferedFd，用于 TLS 加密通信
//! 继承自 BufferedFd，覆写 doRead()/doWrite() 使用 SSL_read/SSL_write
//! SSL 握手由外部（TcpTlsConnector/TcpTlsAcceptor）负责，本类只处理已建立 SSL 连接的 I/O
class BufferedSslFd : public BufferedFd {
  public:
    explicit BufferedSslFd(event::Loop *wp_loop);
    virtual ~BufferedSslFd();

    NONCOPYABLE(BufferedSslFd);
    IMMOVABLE(BufferedSslFd);

    //! 初始化，传入已完成握手的 SSL 对象
    //! 注意：SSL 对象的生命期由本对象管理，析构时会调用 SSL_free()
    bool initialize(Fd fd, SSL *ssl, short events = kReadWrite);

  protected:
    //! 覆写底层 I/O 方法
    virtual ssize_t doReadv(const struct iovec *iov, int iovcnt);
    virtual ssize_t doWrite(const void *data, size_t size) override;

  private:
    ssize_t doRead(void *buffer, size_t size);
    //! 处理 SSL 读写过程中的 WANT_READ/WANT_WRITE（renegotiation）
    void handleSslRenegotiation(int ssl_error, bool is_read_op);

    SSL *ssl_ = nullptr;

    //! renegotiation 状态标记
    bool ssl_read_wants_write_ = false;     //!< SSL_read 返回了 WANT_WRITE
    bool ssl_write_wants_read_ = false;     //!< SSL_write 返回了 WANT_READ
};

}
}
#endif //TBOX_NETWORK_BUFFERED_SSL_FD_H_20260616
