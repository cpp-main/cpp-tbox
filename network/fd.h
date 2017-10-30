#ifndef TBOX_NETWORK_FD_H_20171030
#define TBOX_NETWORK_FD_H_20171030

#include <unistd.h>
#include <sys/uio.h>

namespace tbox {
namespace network {

//! 文件描述符类，封装了对fd的基础操作
class Fd {
  public:
    explicit Fd(int fd);
    virtual ~Fd();

  public:
    int get() const { return fd_; }

    //! 读
    ssize_t read(void *ptr, size_t size) const;
    ssize_t readv(const struct iovec *iov, int iovcnt) const;

    //! 写
    ssize_t write(const void *ptr, size_t size) const;
    ssize_t writev(const struct iovec *iov, int iovcnt) const;

  private:
    int fd_ = -1;
};

}
}

#endif //TBOX_NETWORK_FD_H_20171030
