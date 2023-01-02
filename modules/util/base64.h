#ifndef TBOX_UTIL_BASE64_H_20221229
#define TBOX_UTIL_BASE64_H_20221229

#include <string>

namespace tbox {
namespace util {
namespace base64 {

constexpr size_t EncodeLength(size_t plain_text_length) {
    return (plain_text_length + 2) / 3 * 4;
}

size_t DecodeLength(const char *encode_str, size_t encode_str_len);
size_t DecodeLength(const char *encode_str);
size_t DecodeLength(const std::string &encode_str);


size_t Encode(const uint8_t *in, size_t inlen, char *out, size_t outlen);
std::string Encode(const uint8_t *in, size_t inlen);

size_t Decode(const char *in, size_t inlen, uint8_t *out, size_t outlen);

}
}
}
#endif
