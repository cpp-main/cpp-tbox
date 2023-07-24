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
#include "serializer.h"
#include <cstring>

namespace tbox {
namespace util {

////////////
// 序列化 //
////////////

Serializer::Serializer(void *start, size_t size, Endian endian) :
    type_(kRaw),
    start_((uint8_t *)start),
    size_(size),
    endian_(endian)
{ }

Serializer::Serializer(std::vector<uint8_t> &block, Endian endian) :
    type_(kVector),
    start_((uint8_t *)block.data()),
    p_block_(&block),
    endian_(endian)
{ }

Endian Serializer::setEndian(Endian e)
{
    auto old = endian_;
    endian_ = e;
    return old;
}

bool Serializer::extendSize(size_t need_size)
{
    size_t whole_size = pos_ + need_size;
    if (type_ == kRaw)
        return whole_size <= size_;
    else {
        p_block_->resize(whole_size);
        start_ = p_block_->data();
        return true;
    }
}

bool Serializer::append(uint8_t in)
{
    if (!extendSize(1))
        return false;

    start_[pos_] = in;
    ++pos_;
    return true;
}

bool Serializer::append(uint16_t in)
{
    if (!extendSize(2))
        return false;

    uint8_t *p = start_ + pos_;
    if (endian_ == Endian::kBig) {
        p[1] = in & 0xff;
        in >>= 8; p[0] = in & 0xff;
    } else {
        p[0] = in & 0xff;
        in >>= 8; p[1] = in & 0xff;
    }

    pos_ += 2;
    return true;
}

bool Serializer::append(uint32_t in)
{
    if (!extendSize(4))
        return false;

    uint8_t *p = start_ + pos_;
    if (endian_ == Endian::kBig) {
        p[3] = in & 0xff;
        in >>= 8; p[2] = in & 0xff;
        in >>= 8; p[1] = in & 0xff;
        in >>= 8; p[0] = in & 0xff;
    } else {
        p[0] = in & 0xff;
        in >>= 8; p[1] = in & 0xff;
        in >>= 8; p[2] = in & 0xff;
        in >>= 8; p[3] = in & 0xff;
    }

    pos_ += 4;
    return true;
}

bool Serializer::append(uint64_t in)
{
    if (!extendSize(8))
        return false;

    uint8_t *p = start_ + pos_;
    if (endian_ == Endian::kBig) {
        p[7] = in & 0xff;
        in >>= 8; p[6] = in & 0xff;
        in >>= 8; p[5] = in & 0xff;
        in >>= 8; p[4] = in & 0xff;
        in >>= 8; p[3] = in & 0xff;
        in >>= 8; p[2] = in & 0xff;
        in >>= 8; p[1] = in & 0xff;
        in >>= 8; p[0] = in & 0xff;
    } else {
        p[0] = in & 0xff;
        in >>= 8; p[1] = in & 0xff;
        in >>= 8; p[2] = in & 0xff;
        in >>= 8; p[3] = in & 0xff;
        in >>= 8; p[4] = in & 0xff;
        in >>= 8; p[5] = in & 0xff;
        in >>= 8; p[6] = in & 0xff;
        in >>= 8; p[7] = in & 0xff;
    }

    pos_ += 8;
    return true;
}

bool Serializer::append(const void *p_in, size_t size)
{
    if (!extendSize(size))
        return false;

    uint8_t *p_out = start_ + pos_;
    memcpy(p_out, p_in, size);

    pos_ += size;
    return true;
}

bool Serializer::appendPOD(const void *p, size_t size)
{
    if (!extendSize(size))
        return false;

    uint8_t *p_out = start_ + pos_;
    if (endian_ == Endian::kLittle) {
        memcpy(p_out, p, size);
    } else {
        //! 倒序写入
        const uint8_t *p_in = static_cast<const uint8_t*>(p);
        p_out += size - 1;
        size_t times = size;
        while (times-- > 0)
            *p_out-- = *p_in++;
    }

    pos_ += size;
    return true;
}


//////////////
// 反序列化 //
//////////////

Deserializer::Deserializer(const void *start, size_t size, Endian endian) :
    start_((const uint8_t *)start),
    size_(size),
    endian_(endian),
    pos_(0)
{ }

Endian Deserializer::setEndian(Endian e)
{
    auto old = endian_;
    endian_ = e;
    return old;
}

bool Deserializer::checkSize(size_t need_size) const
{
    return (pos_ + need_size) <= size_;
}

bool Deserializer::set_pos(size_t pos) {
    if (pos < size_) {
        pos_ = pos;
        return true;
    }
    return false;
}

bool Deserializer::fetch(uint8_t &out)
{
    if (!checkSize(1))
        return false;

    const uint8_t *p = start_ + pos_;
    out = p[0];

    ++pos_;
    return true;
}

bool Deserializer::fetch(uint16_t &out)
{
    if (!checkSize(2))
        return false;

    const uint8_t *p = start_ + pos_;
    if (endian_ == Endian::kBig) {
        out = p[0];
        out <<= 8; out |= p[1];
    } else {
        out = p[1];
        out <<= 8; out |= p[0];
    }

    pos_ += 2;
    return true;
}

bool Deserializer::fetch(uint32_t &out)
{
    if (!checkSize(4))
        return false;

    const uint8_t *p = start_ + pos_;
    if (endian_ == Endian::kBig) {
        out = p[0];
        out <<= 8; out |= p[1];
        out <<= 8; out |= p[2];
        out <<= 8; out |= p[3];
    } else {
        out = p[3];
        out <<= 8; out |= p[2];
        out <<= 8; out |= p[1];
        out <<= 8; out |= p[0];
    }

    pos_ += 4;
    return true;
}

bool Deserializer::fetch(uint64_t &out)
{
    if (!checkSize(8))
        return false;

    const uint8_t *p = start_ + pos_;
    if (endian_ == Endian::kBig) {
        out = p[0];
        out <<= 8; out |= p[1];
        out <<= 8; out |= p[2];
        out <<= 8; out |= p[3];
        out <<= 8; out |= p[4];
        out <<= 8; out |= p[5];
        out <<= 8; out |= p[6];
        out <<= 8; out |= p[7];
    } else {
        out = p[7];
        out <<= 8; out |= p[6];
        out <<= 8; out |= p[5];
        out <<= 8; out |= p[4];
        out <<= 8; out |= p[3];
        out <<= 8; out |= p[2];
        out <<= 8; out |= p[1];
        out <<= 8; out |= p[0];
    }

    pos_ += 8;
    return true;
}

bool Deserializer::fetch(void *p, size_t size)
{
    if (!checkSize(size))
        return false;

    const uint8_t *p_in = start_ + pos_;
    memcpy(p, p_in, size);

    pos_ += size;
    return true;
}

bool Deserializer::fetchPOD(void *p, size_t size)
{
    if (!checkSize(size))
        return false;

    const uint8_t *p_in = start_ + pos_;
    if (endian_ == Endian::kLittle) {
        memcpy(p, p_in, size);
    } else {
        //! 倒序读出
        uint8_t *p_out = static_cast<uint8_t*>(p) + size - 1;
        size_t times = size;
        while (times-- > 0)
            *p_out-- = *p_in++;
    }

    pos_ += size;
    return true;
}

const void* Deserializer::fetchNoCopy(size_t size)
{
    if (!checkSize(size))
        return nullptr;

    const uint8_t *p = start_ + pos_;
    pos_ += size;

    return p;
}

bool Deserializer::skip(size_t size) {
    if (!checkSize(size))
        return false;

    pos_ += size;
    return true;
}

}
}

using namespace tbox::util;

Serializer& operator << (Serializer &s, Endian endian) { s.setEndian(endian);  return s; }
Serializer& operator << (Serializer &s, uint8_t in) { s.append(in); return s; }
Serializer& operator << (Serializer &s, int8_t in) { s.append(static_cast<uint8_t>(in)); return s; }
Serializer& operator << (Serializer &s, uint16_t in) { s.append(in); return s; }
Serializer& operator << (Serializer &s, int16_t in) { s.append(static_cast<uint16_t>(in)); return s; }
Serializer& operator << (Serializer &s, uint32_t in) { s.append(in); return s; }
Serializer& operator << (Serializer &s, int32_t in) { s.append(static_cast<uint32_t>(in)); return s; }
Serializer& operator << (Serializer &s, uint64_t in) { s.append(in); return s; }
Serializer& operator << (Serializer &s, int64_t in) { s.append(static_cast<uint64_t>(in)); return s; }
Serializer& operator << (Serializer &s, float in) { s.appendPOD(&in, sizeof(in)); return s; }
Serializer& operator << (Serializer &s, double in) { s.appendPOD(&in, sizeof(in)); return s; }

Deserializer& operator >> (Deserializer &s, Endian endian) { s.setEndian(endian); return s; }
Deserializer& operator >> (Deserializer &s, uint8_t &out) { s.fetch(out); return s; }
Deserializer& operator >> (Deserializer &s, int8_t &out) { s.fetch(*((uint8_t*)&out)); return s; }
Deserializer& operator >> (Deserializer &s, uint16_t &out) { s.fetch(out); return s; }
Deserializer& operator >> (Deserializer &s, int16_t &out) { s.fetch(*((uint16_t*)&out)); return s; }
Deserializer& operator >> (Deserializer &s, uint32_t &out) { s.fetch(out); return s; }
Deserializer& operator >> (Deserializer &s, int32_t &out) { s.fetch(*((uint32_t*)&out)); return s; }
Deserializer& operator >> (Deserializer &s, uint64_t &out) { s.fetch(out); return s; } 
Deserializer& operator >> (Deserializer &s, int64_t &out) { s.fetch(*((uint64_t*)&out)); return s; }
Deserializer& operator >> (Deserializer &s, float &out) { s.fetchPOD(&out, sizeof(out)); return s; }
Deserializer& operator >> (Deserializer &s, double &out) { s.fetchPOD(&out, sizeof(out)); return s; }
