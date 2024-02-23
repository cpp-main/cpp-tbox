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
#include "execute_cmd.h"

#include <unistd.h>
#include <fcntl.h>

#include <tbox/base/log.h>

namespace tbox {
namespace util {

bool ExecuteCmd(const std::string &cmd)
{
    int ret = 0;
    return ExecuteCmd(cmd, ret);
}

bool ExecuteCmd(const std::string &cmd, int &ret)
{
    ret = ::system(cmd.c_str());
#if 0
    LogTrace("system(\"%s\") = %d", cmd.c_str(), ret);
#endif
    if (ret != 0) {
        LogWarn("system(\"%s\") = %d", cmd.c_str(), ret);
        return false;
    }
    return true;
}

bool ExecuteCmd(const std::string &cmd, std::string &result)
{
    int ret = 0;
    return ExecuteCmd(cmd, result, ret);
}

bool ExecuteCmd(const std::string &cmd, std::string &result, int &ret)
{
    auto fp = ::popen(cmd.c_str(), "r");
    if (fp == nullptr) {
        LogWarn("popen(\"%s\") fail", cmd.c_str());
        return false;
    }

    for (;;) {
        char buff[1025];
        auto rsize = ::fread(buff, 1, (sizeof(buff) - 1), fp);
        if (rsize == 0)
            break;
        buff[rsize] = '\0';
        result += buff;
    }

    ret = pclose(fp);
#if 0
    LogTrace("popen(\"%s\") = %d, \"%s\"", cmd.c_str(), ret, buff);
#endif
    if (ret != 0) {
        LogWarn("pclose ret:%d", ret);
        return false;
    }

    return true;
}

}
}
