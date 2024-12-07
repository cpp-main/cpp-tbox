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
#ifndef TBOX_NETWORK_BYTE_STREAM_H_20171102
#define TBOX_NETWORK_BYTE_STREAM_H_20171102

#include <cstddef>
#include <functional>

#include <tbox/util/buffer.h>

namespace tbox {
namespace network {

using Buffer = util::Buffer;

//! 字节流接口
class ByteStream {
  public:
    //! 函数类型定义
    using ReceiveCallback = std::function<void(Buffer&)>;
    using SendCompleteCallback = std::function<void()>;

    //! 设置接收到数据时的回调函数，threshold 为阈值
    virtual void setReceiveCallback(const ReceiveCallback &cb, size_t threshold = 0) = 0;
    virtual void setSendCompleteCallback(const SendCompleteCallback &func) = 0;

    //! 发送数据
    virtual bool send(const void *data_ptr, size_t data_size) = 0;

    //! 绑定一个数据接收者
    //! 当接收到的数据时，数据将流向receiver，作为其输出的数据
    virtual void bind(ByteStream *receiver) = 0;

    //! 解除绑定
    virtual void unbind() = 0;

    //! 获取接收缓冲
    //! WARN: 仅限当场使用，切勿将指针存起来
    virtual Buffer* getReceiveBuffer() = 0;

  protected:
    virtual ~ByteStream() { }
};

}
}

#endif //TBOX_NETWORK_BYTE_STREAM_H_20171102
