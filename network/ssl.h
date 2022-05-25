#ifndef TBOX_NETWORK_SSL_H_20220524
#define TBOX_NETWORK_SSL_H_20220524

#include <openssl/ssl.h>
#include <tbox/base/defines.h>

namespace tbox {
namespace network {

//! 对openssl中的SSL对象进行接口挂装
class Ssl {
  public:
    Ssl();
    Ssl(SSL_CTX *ctx);
    ~Ssl();

    NONCOPYABLE(Ssl);

    Ssl(Ssl &&);
    Ssl& operator = (Ssl &&);

    void swap(Ssl &other);
    void reset();

  public:
    int getFd() const;
    int setFd(int fd) const;

    void setAcceptState() const;
    void setConnectState() const;

    int doHandshake() const;

    int read(void *buf, int num) const;
    int readEx(void *buf, size_t num, size_t *readbytes) const;
    int readEarlyData(void *buf, size_t num, size_t *readbytes) const;

    int peek(void *buf, int num) const;
    int peekEx(void *buf, size_t num, size_t *readbytes) const;

    int write(const void *buf, int num) const;
    int writeEx(const void *buf, size_t num, size_t *written) const;
    int writeEarlyData(const void *buf, size_t num, size_t *written) const;

    int getError(int ret_code) const;

    int shutdown() const;

  public:
    SSL *ssl_ = nullptr;
};

}
}

#endif //TBOX_NETWORK_SSL_H_20220524
