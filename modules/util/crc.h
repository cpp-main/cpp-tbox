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
#ifndef TBOX_UTIL_CRC_H_20230104
#define TBOX_UTIL_CRC_H_20230104

#include <cstdlib>
#include <cstdint>

namespace tbox {
namespace util {

uint16_t CalcCrc16(const void *data_ptr, size_t data_size, uint16_t init_seed = 0xffff);
uint32_t CalcCrc32(const void *data_ptr, size_t data_size, uint32_t init_seed = 0xffffffff);

}
}

#endif //TBOX_UTIL_CRC_H_20230104
