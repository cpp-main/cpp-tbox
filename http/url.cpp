#include "url.h"
#include <sstream>

namespace tbox {
namespace http {

namespace {
const std::string special_chars = R"(#%&+/\=? .:)";
const char *char_to_hex = R"(0123456789abcdef)";

char HexCharToValue(char hex_char)
{
    if (('0' <= hex_char) && (hex_char <= '9'))
        return hex_char - '0';
    else if (('A' <= hex_char) && (hex_char <= 'F'))
        return hex_char - 'A' + 10;
    else if (('a' <= hex_char) && (hex_char <= 'f'))
        return hex_char - 'a' + 10;
    else
        throw std::out_of_range("should be A-Z a-z 0-9");
}
}

std::string LocalToUrl(const std::string &local_str)
{
    std::string url_str;
    url_str.reserve(local_str.size() * 5 / 4); //! 预留1.25倍的空间

    for (char c : local_str) {
        if (special_chars.find(c) != std::string::npos) {
            url_str.push_back('%');
            url_str.push_back(char_to_hex[c >> 4]);
            url_str.push_back(char_to_hex[c & 0xf]);
        } else
            url_str.push_back(c);
    }
    return url_str;
}

std::string UrlToLocal(const std::string &url_str)
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
            } else
                local_str.push_back(c);
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
    return local_str;
}

/**
 * URL格式：<scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<frag>
 */

std::string UrlToString(const Url::Host &host)
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

std::string UrlToString(const Url::Path &path)
{
    std::ostringstream oss;

    oss << path.path;

    for (const auto &item : path.params)
        oss << ';' << LocalToUrl(item.first) << '=' << LocalToUrl(item.second);

    if (!path.query.empty()) {
        oss << '?';
        bool not_first = false;
        for (const auto &item : path.query) {
            if (not_first)
                oss << '&';
            not_first = true;
            oss << LocalToUrl(item.first) << '=' << LocalToUrl(item.second);
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

    oss << UrlToString(url.host);
    oss << UrlToString(url.path);

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

    auto path_start_pos = str.find_first_of('/', pos);
    if (path_start_pos == std::string::npos)
        return false;

    std::string host_str = str.substr(pos, path_start_pos - pos);
    std::string path_str = str.substr(path_start_pos);

    return StringToUrl(host_str, url.host) &&
           StringToUrl(path_str, url.path);
}

bool StringToUrl(const std::string &str, Url::Host &host)
{
    //! <user>:<password>@<host>:<port>
}

bool StringToUrl(const std::string &str, Url::Path &path)
{
    //! /<path>;<params>?<query>#<frag>
}

}
}
