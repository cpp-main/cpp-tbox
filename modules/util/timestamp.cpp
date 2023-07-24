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

}
}


