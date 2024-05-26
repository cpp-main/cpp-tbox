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
#ifndef TBOX_UTIL_BUFFER_H_20171028
#define TBOX_UTIL_BUFFER_H_20171028

#include <stdint.h>

namespace tbox {
namespace util {

/**
 * 缓冲区类
 *
 * buffer_ptr_                        buffer_size_
 *   |                                      |
 *   v                                      V
 *   +----+----------------+----------------+
 *   |    | readable bytes | writable bytes |
 *   +----+----------------+----------------+
 *        ^                ^
 *        |                |
 *    read_index_       write_index_
 *
 * 使用示例：
 *  Buffer b;
 *  b.append("abc", 4);  //! 往缓冲中写入4字节的数据
 *  char buff[10];
 *  b.fetch(buff, 4);    //! 从缓冲中取出4字节的数据
 *
 *  b.ensureWritableSize(10);   //! 预留10个字节
 *  memset(b.writableBegin(), 0xcc, 10);    //! 将该10个字节全置为0xcc
 *  b.hasWritten(10);           //! 标该已写入10个字节
 *
 * \warnning    多线程使用需在外部加锁
 */
class Buffer {
  public:
    static const size_t kInitialSize = 256;

    explicit Buffer(size_t reverse_size = kInitialSize);
    virtual ~Buffer();

    Buffer(const Buffer &other);
    Buffer(Buffer &&other);

    Buffer& operator = (const Buffer &other);
    Buffer& operator = (Buffer &&other);

    void swap(Buffer &other);
    void reset();

  public:
    /**
     * 写缓冲操作
     */

    //! 获取可写空间大小
    inline size_t writableSize() const { return buffer_size_ - write_index_; }

    //! 保障有指定容量的可写空间
    bool ensureWritableSize(size_t write_size);

    //! 获取写首地址
    inline uint8_t* writableBegin() const {
        return (buffer_ptr_ != nullptr) ? (buffer_ptr_ + write_index_) : nullptr;
    }

    //! 标记已写入数据大小
    void hasWritten(size_t write_size);

    //! 往缓冲区追加指定的数据块，返回实现写入的大小
    size_t append(const void *p_data, size_t data_size);

    /**
     * 读缓冲操作
     */

    //! 获取可读区域大小
    inline size_t readableSize() const { return write_index_ - read_index_; }

    //! 获取可读区首地址
    inline uint8_t* readableBegin() const {
        return (buffer_ptr_ != nullptr) ? (buffer_ptr_ + read_index_) : nullptr;
    }

    //! 标记已读数据大小
    void hasRead(size_t read_size);

    //! 标记已读取全部数据
    void hasReadAll();

    //! 从缓冲区读取指定的数据块，返回实际读到的数据大小
    size_t fetch(void *p_buff, size_t buff_size);

    /**
     * 其它
     */
    //! 缩减缓冲多余容量
    void shrink();

  protected:
    void cloneFrom(const Buffer &other);

  private:
    uint8_t *buffer_ptr_  = nullptr; //! 缓冲区地址
    size_t   buffer_size_ = 0;       //! 缓冲区大小

    size_t   read_index_  = 0;       //! 读位置偏移
    size_t   write_index_ = 0;       //! 写位置偏移
};

}
}

#endif //TBOX_UTIL_BUFFER_H_20171028
