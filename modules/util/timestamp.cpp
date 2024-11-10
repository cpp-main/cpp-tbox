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
#include "timestamp.h"
#include <sys/time.h>
#include <time.h>

namespace tbox {
namespace util {

uint32_t GetCurrentSecondsFrom1970()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    return tv.tv_sec;
}

uint64_t GetCurrentMillisecondsFrom1970()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

uint64_t GetCurrentMicrosecondsFrom1970()
{
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    return tv.tv_sec * 1000000 + tv.tv_usec;
}

std::string GetUtcTimeString(uint32_t sec)
{
    time_t ts_sec = sec;
    struct tm tm;
    gmtime_r(&ts_sec, &tm);

    char timestamp_str[20];
    strftime(timestamp_str, sizeof(timestamp_str), "%F %H:%M:%S", &tm);

    return timestamp_str;
}

std::string GetLocalTimeString(uint32_t sec)
{
    time_t ts_sec = sec;
    struct tm tm;
    localtime_r(&ts_sec, &tm);

    char timestamp_str[20];
    strftime(timestamp_str, sizeof(timestamp_str), "%F %H:%M:%S", &tm);

    return timestamp_str;
}

}
}
