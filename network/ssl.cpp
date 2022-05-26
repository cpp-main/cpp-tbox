#include "ssl.h"
#include <cassert>
#include <utility>
#include <tbox/base/log.h>

namespace tbox {
namespace network {

Ssl::Ssl(SSL_CTX *ctx) :
    ssl_(SSL_new(ctx))
{ }

Ssl::~Ssl()
{
    if (ssl_ != nullptr) {
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
    }
}

Ssl::Ssl(Ssl &&other)
{
    ssl_ = other.ssl_;
    other.ssl_ = nullptr;
}

Ssl& Ssl::operator = (Ssl &&other)
{
    if (this != &other) {
        ssl_ = other.ssl_;
        other.ssl_ = nullptr;
    }
    return *this;
}

void Ssl::swap(Ssl &other)
{
    std::swap(ssl_, other.ssl_);
}

int Ssl::getFd() const
{
    return ::SSL_get_fd(ssl_);
}

int Ssl::setFd(int fd) const
{
    if (!::SSL_set_fd(ssl_, fd)) {
        LogWarn("SSL_set_fd() fail");
        return false;
    }
    return true;;
}

void Ssl::setAcceptState() const
{
    ::SSL_set_accept_state(ssl_);
}

void Ssl::setConnectState() const
{
    ::SSL_set_connect_state(ssl_);
}

int Ssl::doHandshake() const
{
    return ::SSL_do_handshake(ssl_);
}

int Ssl::read(void *buf, int num) const
{
    return ::SSL_read(ssl_, buf, num);
}

int Ssl::readEx(void *buf, size_t num, size_t *readbytes) const
{
    return ::SSL_read_ex(ssl_, buf, num, readbytes);
}

int Ssl::readEarlyData(void *buf, size_t num, size_t *readbytes) const
{
    return ::SSL_read_early_data(ssl_, buf, num, readbytes);
}

int Ssl::peek(void *buf, int num) const
{
    return ::SSL_peek(ssl_, buf, num);
}

int Ssl::peekEx(void *buf, size_t num, size_t *readbytes) const
{
    return ::SSL_peek_ex(ssl_, buf, num, readbytes);
}

int Ssl::write(const void *buf, int num) const
{
    return ::SSL_write(ssl_, buf, num);
}

int Ssl::writeEx(const void *buf, size_t num, size_t *written) const
{
    return ::SSL_write_ex(ssl_, buf, num, written);
}

int Ssl::writeEarlyData(const void *buf, size_t num, size_t *written) const
{
    return ::SSL_write_early_data(ssl_, buf, num, written);
}

int Ssl::getError(int ret_code) const
{
    return ::SSL_get_error(ssl_, ret_code);
}

int Ssl::shutdown() const
{
    return ::SSL_shutdown(ssl_);
}

}
}
