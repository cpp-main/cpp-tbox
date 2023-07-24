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
#include "base64.h"

#include <cstring>
#include <tbox/base/assert.h>

namespace tbox {
namespace util {
namespace base64 {

namespace {

#define BASE64_PAD '='

/* BASE 64 encode table */
const char base64en[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
};

/* ASCII order for BASE 64 decode, 255 in unused character */
const uint8_t base64de[] = {
    /* nul, soh, stx, etx, eot, enq, ack, bel, */
    255, 255, 255, 255, 255, 255, 255, 255,

    /*  bs,  ht,  nl,  vt,  np,  cr,  so,  si, */
    255, 255, 255, 255, 255, 255, 255, 255,

    /* dle, dc1, dc2, dc3, dc4, nak, syn, etb, */
    255, 255, 255, 255, 255, 255, 255, 255,

    /* can,  em, sub, esc,  fs,  gs,  rs,  us, */
    255, 255, 255, 255, 255, 255, 255, 255,

    /*  sp, '!', '"', '#', '$', '%', '&', ''', */
    255, 255, 255, 255, 255, 255, 255, 255,

    /* '(', ')', '*', '+', ',', '-', '.', '/', */
    255, 255, 255, 62, 255, 255, 255, 63,

    /* '0', '1', '2', '3', '4', '5', '6', '7', */
    52, 53, 54, 55, 56, 57, 58, 59,

    /* '8', '9', ':', ';', '<', '=', '>', '?', */
    60, 61, 255, 255, 255, 255, 255, 255,

    /* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
    255, 0, 1, 2, 3, 4, 5, 6,

    /* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
    7, 8, 9, 10, 11, 12, 13, 14,

    /* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
    15, 16, 17, 18, 19, 20, 21, 22,

    /* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
    23, 24, 25, 255, 255, 255, 255, 255,

    /* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
    255, 26, 27, 28, 29, 30, 31, 32,

    /* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
    33, 34, 35, 36, 37, 38, 39, 40,

    /* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
    41, 42, 43, 44, 45, 46, 47, 48,

    /* 'x', 'y', 'z', '{', '|', '}', '~', del, */
    49, 50, 51, 255, 255, 255, 255, 255
};
}

size_t Encode(const void *in, size_t inlen, char *out, size_t outlen)
{
    TBOX_ASSERT(in != nullptr);
    TBOX_ASSERT(out != nullptr);

    if (EncodeLength(inlen) > outlen)
        return 0;

    int s = 0;
    uint8_t l = 0;
    size_t w_pos = 0;
    const uint8_t *in_bytes = static_cast<const uint8_t*>(in);

    for (size_t r_pos = 0; r_pos < inlen; r_pos++) {
        uint8_t c = in_bytes[r_pos];

        switch (s) {
            case 0:
                s = 1;
                out[w_pos++] = base64en[(c >> 2) & 0x3F];
                break;
            case 1:
                s = 2;
                out[w_pos++] = base64en[((l & 0x3) << 4) | ((c >> 4) & 0xF)];
                break;
            case 2:
                s = 0;
                out[w_pos++] = base64en[((l & 0xF) << 2) | ((c >> 6) & 0x3)];
                out[w_pos++] = base64en[c & 0x3F];
                break;
        }
        l = c;
    }

    switch (s) {
        case 1:
            out[w_pos++] = base64en[(l & 0x3) << 4];
            out[w_pos++] = BASE64_PAD;
            out[w_pos++] = BASE64_PAD;
            break;
        case 2:
            out[w_pos++] = base64en[(l & 0xF) << 2];
            out[w_pos++] = BASE64_PAD;
            break;
    }

    return w_pos;
}

std::string Encode(const void *in, size_t inlen)
{
    TBOX_ASSERT(in != nullptr);

    std::string base64_str;
    auto base64_len = EncodeLength(inlen);
    base64_str.reserve(base64_len);

    uint8_t l = 0;
    uint8_t s = 0;
    const uint8_t *in_bytes = static_cast<const uint8_t*>(in);

    for (size_t r_pos = 0; r_pos < inlen; ++r_pos) {
        uint8_t c = in_bytes[r_pos];

        switch (s) {
            case 0:
                s = 1;
                base64_str.push_back(base64en[(c >> 2) & 0x3F]);
                break;
            case 1:
                s = 2;
                base64_str.push_back(base64en[((l & 0x3) << 4) | ((c >> 4) & 0xF)]);
                break;
            case 2:
                s = 0;
                base64_str.push_back(base64en[((l & 0xF) << 2) | ((c >> 6) & 0x3)]);
                base64_str.push_back(base64en[c & 0x3F]);
                break;
        }
        l = c;
    }

    switch (s) {
        case 1:
            base64_str.push_back(base64en[(l & 0x3) << 4]);
            base64_str.push_back(BASE64_PAD);
            base64_str.push_back(BASE64_PAD);
            break;
        case 2:
            base64_str.push_back(base64en[(l & 0xF) << 2]);
            base64_str.push_back(BASE64_PAD);
            break;
    }

    return base64_str;
}

size_t Decode(const char *in, size_t inlen, void *out, size_t outlen)
{
    TBOX_ASSERT(in != nullptr);
    TBOX_ASSERT(out != nullptr);

    if (inlen & 0x3)
        return 0;

    if (DecodeLength(in, inlen) > outlen)
        return 0;

    size_t w_pos = 0;
    uint8_t *out_bytes = static_cast<uint8_t*>(out);

    for (size_t r_pos = 0; r_pos < inlen; r_pos++) {
        char in_c = in[r_pos];
        if (in_c == BASE64_PAD)
            break;

        uint8_t c = base64de[int(in_c)];
        if (c == 255)
            return 0;

        switch (r_pos & 0x3) {
            case 0:
                out_bytes[w_pos] = (c << 2) & 0xFF;
                break;
            case 1:
                out_bytes[w_pos++] |= (c >> 4) & 0x3;
                out_bytes[w_pos] = (c & 0xF) << 4;
                break;
            case 2:
                out_bytes[w_pos++] |= (c >> 2) & 0xF;
                out_bytes[w_pos] = (c & 0x3) << 6;
                break;
            case 3:
                out_bytes[w_pos++] |= c;
                break;
        }
    }

    return w_pos;
}

size_t DecodeLength(const char *encode_str, size_t encode_str_len)
{
    if (encode_str_len == 0 || (encode_str_len & 0x3) != 0)
        return 0;

    size_t len = encode_str_len / 4 * 3;
    //! 检查字串尾部的=符号
    if (encode_str[encode_str_len - 1] == BASE64_PAD)
        --len;
    if (encode_str[encode_str_len - 2] == BASE64_PAD)
        --len;
    return len;
}

size_t DecodeLength(const char *encode_str)
{
    return DecodeLength(encode_str, ::strlen(encode_str));
}

size_t DecodeLength(const std::string &encode_str)
{
    return DecodeLength(encode_str.data(), encode_str.length());
}

}
}
}
