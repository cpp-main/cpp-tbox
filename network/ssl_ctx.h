#ifndef TBOX_NETWORK_SSL_CTX_H_20220522
#define TBOX_NETWORK_SSL_CTX_H_20220522

#include <string>
#include <tbox/base/defines.h>
#include <openssl/ssl.h>

namespace tbox {
namespace network {

class Ssl;

class SslCtx {
  public:
    SslCtx();
    ~SslCtx();

    NONCOPYABLE(SslCtx);

    SslCtx(SslCtx &&);
    SslCtx& operator = (SslCtx &&);

    void swap(SslCtx &other);

  public:
    bool useCertificateFile(const std::string &filename, int filetype);
    bool usePrivateKeyFile(const std::string &filename, int filetype);
    bool checkPrivateKey();

    Ssl* newSsl();

  private:
    static int _instance_count_;
    SSL_CTX *ssl_ctx_;
};

}
}

#endif //TBOX_NETWORK_SSL_CTX_H_20220522
