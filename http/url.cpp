#include "url.h"

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

std::string LocalToUrl(const std::string &local_str)
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

std::string UrlToLocal(const std::string &http_str)
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

bool UrlToString(const Url &url, std::string &str)
{
    //TODO
    return false;
}

bool StringToUrl(const std::string &str, Url &url)
{
    //TODO
    return false;
}

}
}
