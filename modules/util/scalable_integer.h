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

/**
 * 可变长整数的存储与读取
 *
 * 在对存储空间的大小要求比较严苛的场景下，如果全按最大值来存储，那么一个整数需要占用8字节的空间。
 * 然而，这只是极少数情况。大部分情况需要存储的数值是很小的，只需要1~2字节就能存储。全部采用8字节
 * 就会有大量的空间浪费。
 * 为避免这种没有必要的空间浪费，便有了根据整数数值大小来决定使用几个字节来存储的方法。
 *
 * 如下：
 * +-------------------------------------+----------------------+
 * | 存储格式                            | 数值范围             |
 * +-------------------------------------+----------------------+
 * | 0XXXXXXX                            | (0 ~ 127)            |
 * | 1XXXXXXX 0XXXXXXX                   | (128 ~ 16511)        |
 * | 1XXXXXXX 0XXXXXXX 0XXXXXXX          | (16512 ~ 2113663)    |
 * | 1XXXXXXX 0XXXXXXX 0XXXXXXX 0XXXXXXX | (211366 ~ 270549119) |
 * | ... 以此类拟 ...                    |                      |
 * +-------------------------------------+----------------------+
 *
 * 每个字节的最高值表示数值是否结束。为1表示未结束，为0则表示结束。
 * 而剩下的7位则以*大端*的方式存储数值。
 *
 * 提示：64位整数，最大占用10字节空间
 */
#ifndef TBOX_UTIL_SCALABLE_INTEGER_H
#define TBOX_UTIL_SCALABLE_INTEGER_H

#include <cstdint>
#include <cstdlib>

namespace tbox {
namespace util {

/**
 * \brief 将整数值存入到内存地址中
 *
 * \param in_value  需要存入的整数值
 * \param buff_ptr  内存地址
 * \param buff_size 内存空间，建议 >= 10
 *
 * \return  =0  失败
 * \return  >0  占用字节数
 */
size_t DumpScalableInteger(uint64_t in_value, void *buff_ptr, size_t buff_size);

/**
 * \brief 内存地址中提取整数值
 *
 * \param buff_ptr  内存地址
 * \param buff_size 内存空间，建议 >= 10
 * \param out_value 提取出的整数值
 *
 * \return  =0  失败
 * \return  >0  占用字节数
 */
size_t ParseScalableInteger(const void *buff_ptr, size_t buff_size, uint64_t &out_value);

}
}

#endif //TBOX_UTIL_SCALABLE_INTEGER_H
