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
#ifndef TBOX_UTIL_TIMESTAMP_H
#define TBOX_UTIL_TIMESTAMP_H

#include <cstdint>
#include <string>

namespace tbox {
namespace util {

//! 获取当前的时间戳，精确到秒
uint32_t GetUtcSeconds();
uint32_t GetCurrentSecondsFrom1970();

//! 获取当前的时间戳，精确到毫秒
uint64_t GetUtcMilliseconds();
uint64_t GetCurrentMillisecondsFrom1970();

//! 获取当前的时间戳，精确到微秒
uint64_t GetUtcMicroseconds();
uint64_t GetCurrentMicrosecondsFrom1970();

//! 获取当前的时间戳，秒 + 微秒
bool GetUtc(uint32_t &sec, uint32_t &usec);

//! 获取指定时间戳的0时区时间字串
std::string GetUtcTimeString(uint32_t utc_sec);
//! 获取当前时间戳的0时区时间字串
std::string GetUtcTimeString();

//! 获取指定时间戳的本地时间字串
std::string GetLocalTimeString(uint32_t utc_sec);
//! 获取当前时间戳的本地时间字串
std::string GetLocalTimeString();

}
}

#endif //TBOX_UTIL_TIMESTAMP_H
