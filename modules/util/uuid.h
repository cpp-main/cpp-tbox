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
#ifndef TBOX_UTIL_UUID_H_20250904
#define TBOX_UTIL_UUID_H_20250904

#include <string>

namespace tbox {
namespace util {

//! UUID生成策略
enum class UUIDGenStratey {
    kRandom,        //!< 通用随机数生成
};

//! 根据策略生成UUID
std::string GenUUID(UUIDGenStratey stratey = UUIDGenStratey::kRandom);

}
}

#endif //TBOX_UTIL_UUID_H_20250904
