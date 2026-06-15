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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "sha1.h"

#include <cstring>

namespace tbox {
namespace crypto {

namespace {

//! SHA-1 常量
constexpr uint32_t K0 = 0x5A827999; //!< 0~19
constexpr uint32_t K1 = 0x6ED9EBA1; //!< 20~39
constexpr uint32_t K2 = 0x8F1BBCDC; //!< 40~59
constexpr uint32_t K3 = 0xCA62C1D6; //!< 60~79

inline uint32_t RotLeft(uint32_t x, uint32_t n) { return (x << n) | (x >> (32 - n)); }
inline uint32_t Ch(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
inline uint32_t Parity(uint32_t x, uint32_t y, uint32_t z) { return x ^ y ^ z; }
inline uint32_t Maj(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }

}

SHA1::SHA1()
{
    state_[0] = 0x67452301;
    state_[1] = 0xEFCDAB89;
    state_[2] = 0x98BADCFE;
    state_[3] = 0x10325476;
    state_[4] = 0xC3D2E1F0;
    count_ = 0;
    buffer_index_ = 0;
}

void SHA1::update(const void *data_ptr, size_t data_len)
{
    if (is_finished_ || data_ptr == nullptr || data_len == 0)
        return;

    const uint8_t *p = static_cast<const uint8_t*>(data_ptr);

    while (data_len > 0) {
        size_t copy_len = 64 - buffer_index_;
        if (copy_len > data_len)
            copy_len = data_len;

        memcpy(buffer_ + buffer_index_, p, copy_len);
        buffer_index_ += copy_len;
        p += copy_len;
        data_len -= copy_len;
        count_ += copy_len;

        if (buffer_index_ == 64) {
            transform(buffer_);
            buffer_index_ = 0;
        }
    }
}

void SHA1::finish(uint8_t digest[20])
{
    if (is_finished_)
        return;

    //! 填充：1 bit of 1 + 0 bits + 64 bit length
    uint64_t total_bits = count_ * 8;

    buffer_[buffer_index_++] = 0x80;

    if (buffer_index_ > 56) {
        //! 需要额外一个块
        while (buffer_index_ < 64)
            buffer_[buffer_index_++] = 0;
        transform(buffer_);
        buffer_index_ = 0;
    }

    //! 填0直到56字节位置
    while (buffer_index_ < 56)
        buffer_[buffer_index_++] = 0;

    //! 写入总长度（大端序）
    buffer_[56] = static_cast<uint8_t>((total_bits >> 56) & 0xFF);
    buffer_[57] = static_cast<uint8_t>((total_bits >> 48) & 0xFF);
    buffer_[58] = static_cast<uint8_t>((total_bits >> 40) & 0xFF);
    buffer_[59] = static_cast<uint8_t>((total_bits >> 32) & 0xFF);
    buffer_[60] = static_cast<uint8_t>((total_bits >> 24) & 0xFF);
    buffer_[61] = static_cast<uint8_t>((total_bits >> 16) & 0xFF);
    buffer_[62] = static_cast<uint8_t>((total_bits >>  8) & 0xFF);
    buffer_[63] = static_cast<uint8_t>((total_bits >>  0) & 0xFF);

    transform(buffer_);

    //! 输出摘要（大端序）
    for (int i = 0; i < 5; ++i) {
        digest[i * 4 + 0] = static_cast<uint8_t>((state_[i] >> 24) & 0xFF);
        digest[i * 4 + 1] = static_cast<uint8_t>((state_[i] >> 16) & 0xFF);
        digest[i * 4 + 2] = static_cast<uint8_t>((state_[i] >>  8) & 0xFF);
        digest[i * 4 + 3] = static_cast<uint8_t>((state_[i] >>  0) & 0xFF);
    }

    is_finished_ = true;
}

void SHA1::transform(const uint8_t block[64])
{
    uint32_t w[80];

    //! 将64字节块扩展为80个32位字（大端序）
    for (int i = 0; i < 16; ++i)
        w[i] = (static_cast<uint32_t>(block[i * 4 + 0]) << 24)
              | (static_cast<uint32_t>(block[i * 4 + 1]) << 16)
              | (static_cast<uint32_t>(block[i * 4 + 2]) <<  8)
              | (static_cast<uint32_t>(block[i * 4 + 3]) <<  0);

    for (int i = 16; i < 80; ++i)
        w[i] = RotLeft(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

    uint32_t a = state_[0];
    uint32_t b = state_[1];
    uint32_t c = state_[2];
    uint32_t d = state_[3];
    uint32_t e = state_[4];

    for (int i = 0; i < 20; ++i) {
        uint32_t temp = RotLeft(a, 5) + Ch(b, c, d) + e + K0 + w[i];
        e = d; d = c; c = RotLeft(b, 30); b = a; a = temp;
    }

    for (int i = 20; i < 40; ++i) {
        uint32_t temp = RotLeft(a, 5) + Parity(b, c, d) + e + K1 + w[i];
        e = d; d = c; c = RotLeft(b, 30); b = a; a = temp;
    }

    for (int i = 40; i < 60; ++i) {
        uint32_t temp = RotLeft(a, 5) + Maj(b, c, d) + e + K2 + w[i];
        e = d; d = c; c = RotLeft(b, 30); b = a; a = temp;
    }

    for (int i = 60; i < 80; ++i) {
        uint32_t temp = RotLeft(a, 5) + Parity(b, c, d) + e + K3 + w[i];
        e = d; d = c; c = RotLeft(b, 30); b = a; a = temp;
    }

    state_[0] += a;
    state_[1] += b;
    state_[2] += c;
    state_[3] += d;
    state_[4] += e;
}

void SHA1::Calc(const void *data_ptr, size_t data_len, uint8_t digest[20])
{
    SHA1 sha1;
    sha1.update(data_ptr, data_len);
    sha1.finish(digest);
}

}
}
