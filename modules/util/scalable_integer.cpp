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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "scalable_integer.h"

namespace tbox {
namespace util {

namespace {

//! 每多1个字节，要牺牲1个bit
constexpr uint64_t k1ByteMin  = 0x0000000000000000;
constexpr uint64_t k1ByteMax  = 0x000000000000007F;

constexpr uint64_t k2ByteMin  = k1ByteMax + 1;
constexpr uint64_t k2ByteMax  = k2ByteMin + 0x0000000000003FFF;

constexpr uint64_t k3ByteMin  = k2ByteMax + 1;
constexpr uint64_t k3ByteMax  = k3ByteMin + 0x00000000001FFFFF;

constexpr uint64_t k4ByteMin  = k3ByteMax + 1;
constexpr uint64_t k4ByteMax  = k4ByteMin + 0x000000000FFFFFFF;

constexpr uint64_t k5ByteMin  = k4ByteMax + 1;
constexpr uint64_t k5ByteMax  = k5ByteMin + 0x00000007FFFFFFFF;

constexpr uint64_t k6ByteMin  = k5ByteMax + 1;
constexpr uint64_t k6ByteMax  = k6ByteMin + 0x000003FFFFFFFFFF;

constexpr uint64_t k7ByteMin  = k6ByteMax + 1;
constexpr uint64_t k7ByteMax  = k7ByteMin + 0x0001FFFFFFFFFFFF;

constexpr uint64_t k8ByteMin  = k7ByteMax + 1;
constexpr uint64_t k8ByteMax  = k8ByteMin + 0x00FFFFFFFFFFFFFF;

constexpr uint64_t k9ByteMin  = k8ByteMax + 1;
constexpr uint64_t k9ByteMax  = k9ByteMin + 0x7FFFFFFFFFFFFFFF;

constexpr uint64_t k10ByteMin = k9ByteMax + 1;
//! 最大10Byte

const uint64_t _min_value_tbl[] = {
    0,
    k1ByteMin, k2ByteMin, k3ByteMin, k4ByteMin,
    k5ByteMin, k6ByteMin, k7ByteMin, k8ByteMin,
    k9ByteMin, k10ByteMin
};

const uint64_t _max_value_tbl[] = {
    0,
    k1ByteMax, k2ByteMax, k3ByteMax, k4ByteMax,
    k5ByteMax, k6ByteMax, k7ByteMax, k8ByteMax, k9ByteMax,
};

}

size_t DumpScalableInteger(uint64_t in_value, void *buff_ptr, size_t buff_size)
{
    //! 算出该整数需要多少个字节来存储
    size_t need_bytes = 1;
    while (need_bytes < 10) {
        if (in_value <= _max_value_tbl[need_bytes])
            break;
        ++need_bytes;
    }

    //! 如果存储空间不够，就直接结束
    if (buff_size < need_bytes)
        return 0;

    //! 计算出要存入的替代数值
    uint64_t store_value = in_value;
    if (need_bytes > 1)
        store_value -= (_min_value_tbl[need_bytes]);

    uint8_t *byte_ptr = static_cast<uint8_t*>(buff_ptr);

    //! 先写入第一个数值，最高位固定为0
    byte_ptr[need_bytes - 1] = 0x7f & store_value;
    store_value >>= 7;

    //! 再写入其它数值，最高位固定为1
    if (need_bytes >= 2) {
        for (int pos = need_bytes - 2; pos >= 0; --pos) {
            byte_ptr[pos] = 0x80 | (store_value & 0x7f);  //! 最高值固定为1
            store_value >>= 7;
        }
    }

    return need_bytes;
}

size_t ParseScalableInteger(const void *buff_ptr, size_t buff_size, uint64_t &out_value)
{
    const uint8_t *byte_ptr = static_cast<const uint8_t*>(buff_ptr);
    size_t read_bytes = 1;
    uint64_t read_value = 0;
    bool is_completed = false;

    //! 从前到后读取替代数值
    for (size_t i = 0; i < buff_size && i <= 10; ++i) {
        uint8_t value = byte_ptr[i];

        read_value <<= 7;
        read_value |= (value & 0x7F);

        if ((value & 0x80) == 0) {
            is_completed = true;
            break;
        }
        ++read_bytes;
    }

    if (!is_completed)
        return 0;

    //! 转换成真实数值
    out_value = _min_value_tbl[read_bytes] + read_value;

    return read_bytes;
}

}
}
