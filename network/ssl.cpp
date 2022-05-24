#include "ssl.h"
#include <tbox/base/log.h>

namespace tbox {
namespace network {

Ssl::Ssl(SSL_CTX *ctx) :
    ssl_(SSL_new(ctx))
{ }

Ssl::~Ssl()
{
    SSL_free(ssl_);
}

int Ssl::getFd() const
{
    LogUndo();
    return 0;
}

int Ssl::setFd(int fd) const
{
    LogUndo();
    return 0;
}

int Ssl::setAcceptState() const
{
    LogUndo();
    return 0;
}

int Ssl::setConnectState() const
{
    LogUndo();
    return 0;
}

int Ssl::doHandshake() const
{
    LogUndo();
    return 0;
}

int Ssl::read(void *buf, int num) const
{
    LogUndo();
    return 0;
}

int Ssl::readEx(void *buf, size_t num, size_t *readbytes) const
{
    LogUndo();
    return 0;
}

int Ssl::readEarlyData(void *buf, size_t num, size_t *readbytes) const
{
    LogUndo();
    return 0;
}

int Ssl::peek(void *buf, int num) const
{
    LogUndo();
    return 0;
}

int Ssl::peekEx(void *buf, size_t num, size_t *readbytes) const
{
    LogUndo();
    return 0;
}

int Ssl::write(const void *buf, int num) const
{
    LogUndo();
    return 0;
}

int Ssl::writeEx(const void *buf, size_t num, size_t *written) const
{
    LogUndo();
    return 0;
}

int Ssl::writeEarlyData(const void *buf, size_t num, size_t *written) const
{
    LogUndo();
    return 0;
}

int Ssl::getError(int ret_code) const
{
    LogUndo();
    return 0;
}

int Ssl::shutdown() const
{
    LogUndo();
    return 0;
}

}
}
