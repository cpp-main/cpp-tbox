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
#include "respond.h"
#include <sstream>

namespace tbox {
namespace http {

bool Respond::isValid() const
{
    return status_code != StatusCode::kUnset && http_ver != HttpVer::kUnset;
}

std::string Respond::toString() const
{
    std::ostringstream oss;
    oss << HttpVerToString(http_ver) << " " << StatusCodeToString(status_code) << CRLF;

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

