#include "ssl_ctx.h"
#include "ssl.h"

#include <tbox/base/log.h>
#include <openssl/err.h>

namespace tbox {
namespace network {

int SslCtx::_instance_count_ = 0;

SslCtx::SslCtx()
{
    if (_instance_count_ == 0) {
        ::SSL_library_init();
        ::SSL_load_error_strings();
        ::ERR_load_BIO_strings();
    }
    ++_instance_count_;

    ssl_ctx_ = ::SSL_CTX_new(SSLv23_method());
}

SslCtx::~SslCtx()
{
    ::SSL_CTX_free(ssl_ctx_);

    --_instance_count_;
    if (_instance_count_ == 0) {
        ERR_free_strings();
    }
}

SslCtx::SslCtx(SslCtx &&other)
{
    ssl_ctx_ = other.ssl_ctx_;
    other.ssl_ctx_ = nullptr;
}

SslCtx& SslCtx::operator = (SslCtx &&other)
{
    if (this != &other) {
        ssl_ctx_ = other.ssl_ctx_;
        other.ssl_ctx_ = nullptr;
    }
    return *this;
}

void SslCtx::swap(SslCtx &other)
{
    std::swap(ssl_ctx_, other.ssl_ctx_);
}

bool SslCtx::useCertificateFile(const std::string &filename, int filetype)
{
    int ret = ::SSL_CTX_use_certificate_file(ssl_ctx_, filename.c_str(), filetype);
    if (ret <= 0) {
        LogWarn("load certificate file %s fail.", filename.c_str());
        return false;
    }
    return true;
}

bool SslCtx::usePrivateKeyFile(const std::string &filename, int filetype)
{
    int ret = ::SSL_CTX_use_PrivateKey_file(ssl_ctx_, filename.c_str(), filetype);
    if (ret <= 0) {
        LogWarn("load private key file %s fail.", filename.c_str());
        return false;
    }
    return true;
}

bool SslCtx::checkPrivateKey()
{
    int ret = ::SSL_CTX_check_private_key(ssl_ctx_);
    if (ret <= 0) {
        LogWarn("check private key fail");
        return false;
    }
    return true;
}

Ssl* SslCtx::newSsl()
{
    return new Ssl(ssl_ctx_);
}

}
}
