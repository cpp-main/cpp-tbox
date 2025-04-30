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
#ifndef TBOX_HTTP_SERVER_ROUTE_KEY_H_20250419
#define TBOX_HTTP_SERVER_ROUTE_KEY_H_20250419

#include "../../common.h"

namespace tbox {
namespace http {
namespace server {

//! 用于存储处理函数的映射
struct RouteKey {
    Method method;
    std::string path;

    bool operator<(const RouteKey& other) const {
        if (method != other.method)
            return method < other.method;
        return path < other.path;
    }
};

}
}
}

#endif // TBOX_HTTP_SERVER_ROUTE_KEY_H_20250419
