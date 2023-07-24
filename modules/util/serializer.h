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
#ifndef TBOX_UTIL_SERIALIZER_H_20220218
#define TBOX_UTIL_SERIALIZER_H_20220218

#include <cstdlib>
#include <cstdint>
#include <vector>

namespace tbox {
namespace util {

enum class Endian { kBig, kLittle };

class Serializer {
  public:
    Serializer(void *start, size_t size, Endian endian = Endian::kBig);     //! 固定大小的缓冲区
    Serializer(std::vector<uint8_t> &block, Endian endian = Endian::kBig);  //! 可变大小的vector<uint8_t>缓冲区

    Endian setEndian(Endian e);

    inline size_t pos() const { return pos_; }

    bool append(uint8_t in);
    bool append(uint16_t in);
    bool append(uint32_t in);
    bool append(uint64_t in);
    bool append(const void *p, size_t s);
    bool appendPOD(const void *p, size_t s);

  protected:
    bool extendSize(size_t size);

  private:
    enum { kRaw, kVector } type_;
    uint8_t *start_;
    union {
        size_t  size_;
        std::vector<uint8_t> *p_block_;
    };
    Endian   endian_;
    size_t   pos_ = 0;
};

class Deserializer {
  public:
    Deserializer(const void *start, size_t size, Endian endian = Endian::kBig);

    Endian setEndian(Endian e);

    inline size_t pos() const { return pos_; }
    bool set_pos(size_t pos);

    inline const uint8_t* start() const { return start_; }
    inline size_t size() const { return size_; }

    inline const uint8_t* ptr() const { return start_ + pos_; }

    bool fetch(uint8_t &out);
    bool fetch(uint16_t &out);
    bool fetch(uint32_t &out);
    bool fetch(uint64_t &out);
    bool fetch(void *p, size_t s);
    bool fetchPOD(void *p, size_t s);
    const void* fetchNoCopy(size_t s);

    bool skip(size_t s);

    bool checkSize(size_t size) const;

  private:
    const uint8_t *start_;
    size_t   size_;
    Endian   endian_;
    size_t   pos_ = 0;
};

}
}

//! 流式操作

tbox::util::Serializer& operator << (tbox::util::Serializer &s, tbox::util::Endian endian);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, uint8_t in);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, int8_t in);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, uint16_t in);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, int16_t in);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, uint32_t in);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, int32_t in);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, uint64_t in);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, int64_t in);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, float in);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, double in);

tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, tbox::util::Endian endian);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, uint8_t &out);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, int8_t &out);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, uint16_t &out);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, int16_t &out);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, uint32_t &out);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, int32_t &out);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, uint64_t &out);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, int64_t &out);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, float &out);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, double &out);

#endif //TBOX_UTIL_SERIALIZER_H_20220218
