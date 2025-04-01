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
/**
 * 对标准输入输出进行封装，使之更好用
 */
#ifndef TBOX_NETWORK_STDIO_STREAM_H_20171103
#define TBOX_NETWORK_STDIO_STREAM_H_20171103

#include <tbox/base/defines.h>
#include <tbox/event/loop.h>

#include "byte_stream.h"
#include "buffered_fd.h"

namespace tbox {
namespace network {

class StdinStream : public ByteStream {
  public:
    explicit StdinStream(event::Loop *wp_loop);

    NONCOPYABLE(StdinStream);
    IMMOVABLE(StdinStream);

  public:
    virtual void setReceiveCallback(const ReceiveCallback &cb, size_t threshold) override;
    virtual void setSendCompleteCallback(const SendCompleteCallback &) override { }
    virtual bool send(const void *, size_t) override { return false; }
    virtual void bind(ByteStream *receiver) override { buff_fd_.bind(receiver); }
    virtual void unbind() override { buff_fd_.unbind(); }
    virtual Buffer* getReceiveBuffer() override { return buff_fd_.getReceiveBuffer(); }

    bool initialize();

    bool enable();
    bool disable();

  private:
    BufferedFd buff_fd_;
};

class StdoutStream : public ByteStream {
  public:
    explicit StdoutStream(event::Loop *wp_loop);

    NONCOPYABLE(StdoutStream);
    IMMOVABLE(StdoutStream);

  public:
    virtual void setReceiveCallback(const ReceiveCallback &, size_t) override { }
    virtual void setSendCompleteCallback(const SendCompleteCallback &cb) override;
    virtual bool send(const void *data_ptr, size_t data_size) override;
    virtual void bind(ByteStream *) override { }
    virtual void unbind() override { }
    virtual Buffer* getReceiveBuffer() override { return nullptr; }

    bool initialize();

    bool enable();
    bool disable();

  private:
    BufferedFd buff_fd_;
};

class StdioStream : public ByteStream {
  public:
    explicit StdioStream(event::Loop *wp_loop);

    NONCOPYABLE(StdioStream);
    IMMOVABLE(StdioStream);

  public:
    virtual void setReceiveCallback(const ReceiveCallback &cb, size_t threshold) override;
    virtual void setSendCompleteCallback(const SendCompleteCallback &cb) override;
    virtual bool send(const void *data_ptr, size_t data_size) override;
    virtual void bind(ByteStream *receiver) override { in_buff_fd_.bind(receiver); }
    virtual void unbind() override { in_buff_fd_.unbind(); }
    virtual Buffer* getReceiveBuffer() override { return in_buff_fd_.getReceiveBuffer(); }

    bool initialize();

    bool enable();
    bool disable();

  private:
    BufferedFd in_buff_fd_;
    BufferedFd out_buff_fd_;
};

}
}

#endif //TBOX_NETWORK_STDIO_STREAM_H_20171103
