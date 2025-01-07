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
#ifndef TBOX_NETWORK_BUFFERED_FD_H_20171030
#define TBOX_NETWORK_BUFFERED_FD_H_20171030

#include <functional>
#include <tbox/event/forward.h>
#include <tbox/base/defines.h>
#include <tbox/util/fd.h>

#include "byte_stream.h"

namespace tbox {
namespace network {

using Fd = util::Fd;

class BufferedFd : public ByteStream {
  public:
    explicit BufferedFd(event::Loop *wp_loop);
    virtual ~BufferedFd();

    NONCOPYABLE(BufferedFd);
    IMMOVABLE(BufferedFd);

  public:
    using SendCompleteCallback  = std::function<void()>;
    using ReadZeroCallback      = std::function<void()>;
    using ErrorCallback         = std::function<void(int)>;

    enum class State {
        kEmpty,     //! 未初始化
        kInited,    //! 已初始化
        kRunning    //! 正在运行
    };

    enum {
        kReadOnly  = 0x01,
        kWriteOnly = 0x02,
        kReadWrite = 0x03,
    };

    //! 初始化，并指定发送或是接收功能
    bool initialize(Fd fd, short events = kReadWrite);

    //! 设置完成了当前数据发送时的回调函数
    void setSendCompleteCallback(const SendCompleteCallback &func) { send_complete_cb_ = func; }
    //! 设置当读到0字节数据时回调函数
    void setReadZeroCallback(const ReadZeroCallback &func) { read_zero_cb_ = func; }
    //! 设置当读时遇到错误时的回调函数
    void setReadErrorCallback(const ErrorCallback &func) { read_error_cb_ = func; }
    //! 设置当写时遇到错误时的回调函数
    void setWriteErrorCallback(const ErrorCallback &func) { write_error_cb_ = func; }

    //! 实现 ByteStream 的接口
    virtual void setReceiveCallback(const ReceiveCallback &func, size_t threshold) override;
    virtual bool send(const void *data_ptr, size_t data_size) override;
    virtual void bind(ByteStream *receiver) override { wp_receiver_ = receiver; }
    virtual void unbind() override { wp_receiver_ = nullptr; }
    virtual Buffer* getReceiveBuffer() { return &recv_buff_; }

    //! 启动与关闭内部事件驱动机制
    bool enable();
    bool disable();

    //! 缩减缓冲，防止长期占用大空间内存
    void shrinkRecvBuffer();
    void shrinkSendBuffer();

    inline Fd fd() const { return fd_; }
    inline State state() const { return state_; }

  private:
    void onReadCallback(short);
    void onWriteCallback(short);

  private:
    event::Loop *wp_loop_ = nullptr;    //! 事件驱动

    Fd fd_;
    State state_ = State::kEmpty;

    event::FdEvent *sp_read_event_  = nullptr;
    event::FdEvent *sp_write_event_ = nullptr;

    Buffer send_buff_;
    Buffer recv_buff_;

    ReceiveCallback         receive_cb_;
    SendCompleteCallback    send_complete_cb_;
    ReadZeroCallback        read_zero_cb_;
    ErrorCallback           read_error_cb_;
    ErrorCallback           write_error_cb_;

    ByteStream  *wp_receiver_ = nullptr;

    size_t  receive_threshold_ = 0;
    int     cb_level_ = 0;
};

}
}

#endif //TBOX_NETWORK_BUFFERED_FD_H_20171030
