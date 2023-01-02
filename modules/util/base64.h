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

/**
 * \brief   编码，写到指定缓存
 *
 * \param   in      原文地址
 * \param   inlen   原文长度
 * \param   out     Base64输出地址
 * \param   outlen  Base64输出空间大小
 *
 * \return  >0      实际Base64输出大小
 * \return  =0      outlen不够
 *
 * \note    不会主动加字串结束符
 */
size_t Encode(const uint8_t *in, size_t inlen, char *out, size_t outlen);

/**
 * \brief   编码，返回std::string的base64字串
 *
 * \param   in      原文地址
 * \param   inlen   原文长度
 *
 * \return  std::string base64字串
 */
std::string Encode(const uint8_t *in, size_t inlen);

/**
 * \brief   解码，写到指定缓存
 *
 * \param   in      Base64地址
 * \param   inlen   Base64长度
 * \param   out     原文输出地址
 * \param   outlen  原文输出空间大小
 *
 * \return  >0      实际明文输出大小
 * \return  =0      outlen不够
 */
size_t Decode(const char *in, size_t inlen, uint8_t *out, size_t outlen);

}
}
}
#endif
