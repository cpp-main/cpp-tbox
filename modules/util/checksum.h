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
#ifndef TBOX_UTIL_CHECKSUM_H_20230104
#define TBOX_UTIL_CHECKSUM_H_20230104

#include <cstdlib>
#include <cstdint>

namespace tbox {
namespace util {

/**
 * \brief       计算8位和校验
 *
 * \param data_ptr      需要计算和校验的数据地址
 * \param data_size     需要计算和校验的数据大小
 *
 * \return      和校验值
 */
uint8_t CalcCheckSum8(const void *data_ptr, size_t data_size);

/**
 * \brief       计算16位和校验
 *
 * \param data_ptr      需要计算和校验的数据地址
 * \param data_size     需要计算和校验的数据大小
 *
 * \return      和校验值
 */
uint16_t CalcCheckSum16(const void *data_ptr, size_t data_size);

}
}

#endif //TBOX_UTIL_CHECKSUM_H_20230104
