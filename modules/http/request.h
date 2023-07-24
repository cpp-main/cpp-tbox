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
#ifndef TBOX_HTTP_REQUEST_H_20220501
#define TBOX_HTTP_REQUEST_H_20220501

#include "common.h"
#include "url.h"

namespace tbox {
namespace http {

//! 请求
struct Request {
    Method  method = Method::kUnset;
    HttpVer http_ver = HttpVer::kUnset;
    Url::Path url;
    Headers headers;
    std::string body;

    bool isValid() const;
    std::string toString() const;
};

}
}

#endif //TBOX_HTTP_REQUEST_H_20220501
