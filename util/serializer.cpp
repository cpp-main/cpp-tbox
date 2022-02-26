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

    uint8_t *p = start_ + pos_;
    memcpy(p, p_in, size);

    pos_ += size;
    return true;
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

//////////////
// 反序列化 //
//////////////

Deserializer::Deserializer(const void *start, size_t size, Endian endian) :
    start_((const uint8_t *)start),
    size_(size),
    endian_(endian),
    pos_(0)
{ }

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

bool Deserializer::fetch(void *p_out, size_t size)
{
    if (!checkSize(size))
        return false;

    const uint8_t *p = start_ + pos_;
    memcpy(p_out, p, size);

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

bool Deserializer::checkSize(size_t need_size) const
{
    return (pos_ + need_size) <= size_;
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
Serializer& operator << (Serializer &s, float in) { s.append(*reinterpret_cast<uint32_t*>(&in)); return s; }
Serializer& operator << (Serializer &s, double in) { s.append(*reinterpret_cast<uint64_t*>(&in)); return s; }

Deserializer& operator >> (Deserializer &s, Endian endian) { s.setEndian(endian); return s; }
Deserializer& operator >> (Deserializer &s, uint8_t &out) { s.fetch(out); return s; }
Deserializer& operator >> (Deserializer &s, int8_t &out) { s.fetch(*((uint8_t*)&out)); return s; }
Deserializer& operator >> (Deserializer &s, uint16_t &out) { s.fetch(out); return s; }
Deserializer& operator >> (Deserializer &s, int16_t &out) { s.fetch(*((uint16_t*)&out)); return s; }
Deserializer& operator >> (Deserializer &s, uint32_t &out) { s.fetch(out); return s; }
Deserializer& operator >> (Deserializer &s, int32_t &out) { s.fetch(*((uint32_t*)&out)); return s; }
Deserializer& operator >> (Deserializer &s, uint64_t &out) { s.fetch(out); return s; } 
Deserializer& operator >> (Deserializer &s, int64_t &out) { s.fetch(*((uint64_t*)&out)); return s; }
Deserializer& operator >> (Deserializer &s, float &out) { s.fetch(*reinterpret_cast<uint32_t*>(&out)); return s; }
Deserializer& operator >> (Deserializer &s, double &out) { s.fetch(*reinterpret_cast<uint64_t*>(&out)); return s; }
