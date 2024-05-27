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
#ifndef TBOX_UTIL_FD_H_20171030
#define TBOX_UTIL_FD_H_20171030

#include <unistd.h>
#include <functional>
#include <sys/uio.h>

namespace tbox {
namespace util {

//! 文件描述符类，封装了对fd的基础操作
class Fd {
  public:
    using CloseFunc = std::function<void(int)>;

    Fd();
    Fd(int fd);
    Fd(int fd, const CloseFunc &close_func);
    virtual ~Fd();

    Fd(const Fd& other);
    Fd& operator = (const Fd& other);

    Fd(Fd&& other);
    Fd& operator = (Fd&& other);

    void swap(Fd &other);
    void reset();   //! 重置本Fd

    void close();   //! 提前关闭资源，无论是否还有其它Fd对象引用

    inline bool isNull() const { return detail_ == nullptr || detail_->fd == -1; }

  public:   //! 创建函数
    static Fd Open(const char *filename, int flags);

  public:
    //! 获取文件描述符的值。注意谨慎使用
    inline int get() const { return (detail_ != nullptr) ? detail_->fd : -1; }

    //! 读
    ssize_t read(void *ptr, size_t size) const;
    ssize_t readv(const struct iovec *iov, int iovcnt) const;

    //! 写
    ssize_t write(const void *ptr, size_t size) const;
    ssize_t writev(const struct iovec *iov, int iovcnt) const;

    //! 其它
    void setNonBlock(bool enable) const;    //! 开启或关闭非阻塞选项
    bool isNonBlock() const;        //! 检查是否非阻塞
    void setCloseOnExec() const;    //! 设置Exec时关闭选项

  protected:
    struct Detail {
        int fd = -1;
        int ref_count = 1;
        CloseFunc close_func;
    };

  private:
    Detail *detail_ = nullptr;
};

}
}

#endif //TBOX_UTIL_FD_H_20171030
