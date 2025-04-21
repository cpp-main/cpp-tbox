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
#include "url.h"
#include <sstream>
#include <tbox/util/string.h>

namespace tbox {
namespace http {

namespace {
//! 根据 RFC3986, https://www.rfc-editor.org/rfc/rfc3986
const std::string full_special_chars = R"( +&=<>"#,%{}|\^[]`;?:@$/.)";
const std::string path_special_chars = R"( +&=<>"#,%{}|\^[]`;?:@$)";
const char *char_to_hex = R"(0123456789ABCDEF)";

char HexCharToValue(char hex_char)
{
    if (('0' <= hex_char) && (hex_char <= '9'))
        return hex_char - '0';
    else if (('A' <= hex_char) && (hex_char <= 'F'))
        return hex_char - 'A' + 10;
    else if (('a' <= hex_char) && (hex_char <= 'f'))
        return hex_char - 'a' + 10;
    else
        throw std::runtime_error("should be A-Z a-z 0-9");
}
}

std::string UrlEncode(const std::string &local_str, bool path_mode)
{
    const auto& special_chars = path_mode ? path_special_chars : full_special_chars;

    std::string url_str;
    url_str.reserve(local_str.size() * 5 / 4); //! 预留1.25倍的空间

    for (char c : local_str) {
        //! 如果非ASCII或是特殊字符
        if (special_chars.find(c) != std::string::npos || !std::isprint(c)) {
            unsigned char uc = static_cast<unsigned char>(c);
            url_str.push_back('%');
            url_str.push_back(char_to_hex[uc >> 4]);
            url_str.push_back(char_to_hex[uc & 0xf]);
        } else
            url_str.push_back(c);
    }
    return url_str;
}

std::string UrlDecode(const std::string &url_str)
{
    std::string local_str;
    local_str.reserve(url_str.size());

    enum class State {
        kNone,
        kStart,
        kHalfHex,
    };

    char tmp = 0;
    State state = State::kNone;

    for (char c : url_str) {
        if (state == State::kNone) {
            if (c == '%') {
                state = State::kStart;
                tmp = 0;
            } else if (c == '+') {
                local_str.push_back(' ');
            } else if (c == '#') {
                break; //! 结束
            } else {
                local_str.push_back(c);
            }
        } else {
            if (state == State::kStart) {
                tmp = HexCharToValue(c) << 4;
                state = State::kHalfHex;
            } else if (state == State::kHalfHex) {
                tmp |= HexCharToValue(c);
                state = State::kNone;
                local_str.push_back(tmp);
            }
        }
    }

    if (state != State::kNone)
        throw std::runtime_error("url imcomplete");

    return local_str;
}

/**
 * URL格式：<scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<frag>
 */

std::string UrlHostToString(const Url::Host &host)
{
    std::ostringstream oss;

    if (!host.user.empty()) {
        oss << host.user;
        if (!host.password.empty())
            oss << ':' << host.password;
        oss << '@';
    }

    oss << host.host;
    if (host.port != 0)
        oss << ':' << host.port;

    return oss.str();
}

std::string UrlPathToString(const Url::Path &path)
{
    std::ostringstream oss;

    oss << UrlEncode(path.path, true);

    for (const auto &item : path.params)
        oss << ';' << UrlEncode(item.first) << '=' << UrlEncode(item.second);

    if (!path.query.empty()) {
        oss << '?';
        bool not_first = false;
        for (const auto &item : path.query) {
            if (not_first)
                oss << '&';
            not_first = true;
            oss << UrlEncode(item.first) << '=' << UrlEncode(item.second);
        }
    }

    if (!path.frag.empty())
        oss << '#' << path.frag;

    return oss.str();
}

std::string UrlToString(const Url &url)
{
    std::ostringstream oss;

    if (!url.scheme.empty())
        oss << url.scheme << "://";

    oss << UrlHostToString(url.host);
    oss << UrlPathToString(url.path);

    return oss.str();
}

bool StringToUrl(const std::string &str, Url &url)
{
    auto pos = str.find("://");
    if (pos != std::string::npos) {
        url.scheme = str.substr(0, pos);
        pos += 3;
    } else {
        pos = 0;
    }

    std::string host_str;
    std::string path_str;

    //! 将剩下的 url 拆分成两部分 host + path，分别进行解析
    auto path_start_pos = str.find_first_of('/', pos);
    if (path_start_pos != std::string::npos) {
        host_str = str.substr(pos, path_start_pos - pos);
        path_str = str.substr(path_start_pos);
    } else {
        host_str = str.substr(pos);
        path_str = "/";
    }

    return StringToUrlHost(host_str, url.host) &&
           StringToUrlPath(path_str, url.path);
}

bool StringToUrlHost(const std::string &str, Url::Host &host)
{
    //! 解析：<user>:<password>@<host>:<port>

    //! 先找@。如果找到了，就分成 user:password 与 host:port 两部分
    //! 如果没有找到，则说明只有 host:port
    auto at_pos = str.find_first_of('@');
    auto host_start_pose = 0;
    try {
        if (at_pos != std::string::npos) {
            auto colon_pos = str.find_first_of(':');
            if (colon_pos == std::string::npos || colon_pos > at_pos) {
                //! 说明没有 password
                host.user = UrlDecode(str.substr(0, at_pos));
                host.password.clear();
            } else {
                host.user = UrlDecode(str.substr(0, colon_pos));
                host.password = UrlDecode(str.substr(colon_pos + 1, at_pos - colon_pos - 1));
            }
            host_start_pose = at_pos + 1;
        }

        auto colon_pos = str.find_first_of(':', host_start_pose);
        if (colon_pos == std::string::npos) {
            //! 说明没有 password
            host.host = UrlDecode(str.substr(host_start_pose));
            host.port = 0;
        } else {
            host.host = UrlDecode(str.substr(host_start_pose, colon_pos - host_start_pose));
            host.port = std::stoi(str.substr(colon_pos + 1));
        }
    } catch (const std::exception &) {
        return false;
    }

    return true;
}

bool StringToUrlPath(const std::string &str, Url::Path &path)
{
    //! /<path>;<params>?<query>#<frag>

    if (str.empty() || str[0] != '/')
        return false;

    auto semi_pos = str.find_first_of(';');
    auto query_pos = str.find_first_of('?');
    auto pound_pos = str.find_first_of('#');

    auto path_end_pos = std::string::npos;
    if (semi_pos != std::string::npos)
        path_end_pos = semi_pos;
    else if (query_pos != std::string::npos)
        path_end_pos = query_pos;
    else if (pound_pos != std::string::npos)
        path_end_pos = pound_pos;

    try {
        path.path = UrlDecode(str.substr(0, path_end_pos));

        if (semi_pos != std::string::npos) {
            //! 说明有 params
            auto params_end_pos = std::string::npos;
            if (query_pos != std::string::npos)
                params_end_pos = query_pos;
            else if (pound_pos != std::string::npos)
                params_end_pos = pound_pos;

            auto params_str = str.substr(semi_pos + 1, params_end_pos - semi_pos - 1);
            std::vector<std::string> params_vec;
            util::string::Split(params_str, ";", params_vec);
            for (const auto &param_str : params_vec) {
                std::vector<std::string> kv;
                util::string::Split(param_str, "=", kv);
                if (kv.size() != 2 || kv[0].empty())
                    return false;
                path.params[UrlDecode(kv[0])] = UrlDecode(kv[1]);
            }
        }

        if (query_pos != std::string::npos) {
            //! 说明有 query
            auto querys_end_pos = std::string::npos;
            if (pound_pos != std::string::npos)
                querys_end_pos = pound_pos;

            auto querys_str = str.substr(query_pos + 1, querys_end_pos - query_pos - 1);
            std::vector<std::string> querys_vec;
            util::string::Split(querys_str, "&", querys_vec);
            for (const auto &query_str : querys_vec) {
                std::vector<std::string> kv;
                util::string::Split(query_str, "=", kv);
                if (kv.size() != 2 || kv[0].empty())
                    return false;
                path.query[UrlDecode(kv[0])] = UrlDecode(kv[1]);
            }
        }

        if (pound_pos != std::string::npos) {
            //! 说明有 frag
            path.frag = UrlDecode(str.substr(pound_pos + 1));
        }

    } catch (const std::exception &) {
        return false;
    }

    return true;
}

}
}
