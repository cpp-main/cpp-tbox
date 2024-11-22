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
#include "loop.h"
#include <tbox/base/log.h>

#include "engines/select/loop.h"

#if HAVE_EPOLL
#include "engines/epoll/loop.h"
#endif

namespace tbox {
namespace event {

Loop* Loop::New()
{
#if HAVE_EPOLL
    return new EpollLoop;
#else
    return new SelectLoop;
#endif
}

Loop* Loop::New(const std::string &engine_type)
{
    if (engine_type == "select")
        return new SelectLoop;
#if HAVE_EPOLL
    else if (engine_type == "epoll")
        return new EpollLoop;
#endif

    return nullptr;
}

std::vector<std::string> Loop::Engines()
{
    std::vector<std::string> types;

#if HAVE_EPOLL
    types.push_back("epoll");
#endif
    types.push_back("select");
    return types;
}

}
}
