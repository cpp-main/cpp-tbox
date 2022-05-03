#include "common.h"
#include <exception>

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

std::string LocalToHttp(const std::string &local_str)
{
    std::string http_str;
    http_str.reserve(local_str.size() * 5 / 4); //! 预留1.25倍的空间

    for (char c : local_str) {
        if (special_chars.find(c) != std::string::npos) {
            http_str.push_back('%');
            http_str.push_back(char_to_hex[c >> 4]);
            http_str.push_back(char_to_hex[c & 0xf]);
        } else
            http_str.push_back(c);
    }
    return http_str;
}

std::string HttpToLocal(const std::string &http_str)
{
    std::string local_str;
    local_str.reserve(http_str.size());

    enum class State {
        kNone,
        kStart,
        kHalfHex,
    };

    char tmp = 0;
    State state = State::kNone;

    for (char c : http_str) {
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

}
}

std::ostream& operator << (std::ostream &oss, tbox::http::HttpVer ver)
{
    const char* tbl[] = { "HTTP/1.0", "HTTP/1.1", "HTTP/1.2" };
    int index = static_cast<int>(ver);
    if (index > 0 && index < static_cast<int>(tbox::http::HttpVer::kMax))
        oss << tbl[index - 1];
    return oss;
}

std::ostream& operator << (std::ostream &oss, tbox::http::Method method)
{
    const char* tbl[] = { "GET", "HEAD", "PUT", "POST", "DELETE" };
    int index = static_cast<int>(method);
    if (index > 0 && index < static_cast<int>(tbox::http::Method::kMax))
        oss << tbl[index - 1];
    return oss;
}

std::ostream& operator << (std::ostream &oss, tbox::http::StatusCode code)
{
    const char* tbl[] = {
        "200 OK",
        "201 Created",
        "202 Accepted",
        "203 Non-Authoritative Information",
        "204 No Content",
        "205 Reset Content",
        "206 Partial Content",

        "300 Multiple Choices",
        "301 Moved Permanently",
        "302 Found",
        "303 See Other",
        "304 Not Modified",
        "305 Use Proxy",
        "307 Temporary Redirect",

        "400 Bad Request",
        "401 Unauthorized",
        "402 Payment Required",
        "403 Forbidden",
        "404 Not Found",
        "405 Method Not Allowed",
        "406 Not Acceptable",
        "407 Proxy Authentication Required",
        "408 Request Timeout",
        "409 Conflict",
        "410 Gone",
        "411 Length Required",
        "412 Precondition Failed",
        "413 Request Entity Too Large",
        "414 Request URI Too Long",
        "415 Unsupported Media Type",
        "416 Requested Range Not Satisfiable",
        "417 Expectation Failed",

        "500 Internal Server Error",
        "501 Not Implemented",
        "502 Bad Gateway",
        "503 Service Unavailable",
        "504 Gateway Timeout",
        "505 HTTP Version Not Supported",
    };

    int index = static_cast<int>(code);
    if (index > 0 && index < static_cast<int>(tbox::http::StatusCode::kMax))
        oss << tbl[index - 1];
    return oss;
}


