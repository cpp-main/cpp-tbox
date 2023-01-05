#ifndef TBOX_CRYPTO_BASE64_H_20221229
#define TBOX_CRYPTO_BASE64_H_20221229
namespace tbox {
namespace crypto {

class Base64 {
  public:
    static unsigned int Encode(const unsigned char *in, unsigned int inlen, char *out);
    static unsigned int Decode(const char *in, unsigned int inlen, unsigned char *out);
};

}
}
#endif
