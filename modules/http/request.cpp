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
#include "request.h"
#include <tbox/base/defines.h>
#include <sstream>

namespace tbox {
namespace http {

bool Request::isValid() const
{
    return method != Method::kUnset && http_ver != HttpVer::kUnset && !url.path.empty();
}

std::string Request::toString() const
{
    std::ostringstream oss;
    oss << MethodToString(method) << " " << UrlPathToString(url) << " " << HttpVerToString(http_ver) << CRLF;

    bool has_content_length = false;
    for (auto &head : headers) {
        oss << head.first << ": " << head.second << CRLF;
        if (head.first == "Content-Length")
            has_content_length = true;
    }

    if (!has_content_length)
        oss << "Content-Length: " << body.length() << CRLF;

    oss << CRLF;
    oss << body;

    return oss.str();
}

}
}
