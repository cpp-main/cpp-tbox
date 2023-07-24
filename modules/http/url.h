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
#ifndef TBOX_HTTP_URL_H_20220512
#define TBOX_HTTP_URL_H_20220512

#include <string>
#include <map>
#include <cstdint>

namespace tbox {
namespace http {

/**
 * URL格式：<scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<frag>
 */
struct Url {
    std::string scheme;

    struct Host {
        std::string user;
        std::string password;
        std::string host;
        uint16_t    port = 0;
    };

    struct Path {
        using StringMap = std::map<std::string, std::string>;
        std::string path;
        StringMap   params;
        StringMap   query;
        std::string frag;
    };

    Host host;
    Path path;
};

std::string UrlEncode(const std::string &local_str, bool path_mode = false);
std::string UrlDecode(const std::string &url_str);

std::string UrlToString(const Url &url);
std::string UrlHostToString(const Url::Host &host);
std::string UrlPathToString(const Url::Path &path);

bool StringToUrl(const std::string &str, Url &url);
bool StringToUrlHost(const std::string &str, Url::Host &host);
bool StringToUrlPath(const std::string &str, Url::Path &path);

}
}

#endif //TBOX_HTTP_URL_H_20220512
