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
#include "string.h"

#include <iomanip>
#include <sstream>
#include <algorithm>

namespace tbox {
namespace util {
namespace string {

size_t Split(const std::string &src_str, const std::string sep, std::vector<std::string> &str_vec)
{
    size_t start_pos = 0;
    size_t end_pos = 0;
    str_vec.clear();

    while (true) {
        end_pos = src_str.find(sep, start_pos);
        const std::string &str_chip = src_str.substr(start_pos, end_pos - start_pos);
        str_vec.push_back(str_chip);
        if (end_pos == std::string::npos)
            break;
        start_pos = end_pos + sep.size();
    }

    return str_vec.size();
}

size_t SplitBySpace(const std::string &src_str, std::vector<std::string> &str_vec)
{
    size_t start_pos = 0;
    size_t end_pos = 0;
    str_vec.clear();

    while (true) {
        start_pos = src_str.find_first_not_of(" \t", end_pos);
        if (start_pos == std::string::npos)
            break;
        end_pos = src_str.find_first_of(" \t", start_pos);
        const std::string &str_chip = src_str.substr(start_pos, end_pos - start_pos);
        str_vec.push_back(str_chip);
        if (end_pos == std::string::npos)
            break;
    }

    return str_vec.size();
}

std::string Join(const std::vector<std::string> &str_vec, const std::string &delimiter)
{
    std::string tmp;

    if (!str_vec.empty()) {
        //! 计算预留空间
        size_t reserve_length = delimiter.length() * (str_vec.size() - 1);
        for (const auto &str : str_vec)
            reserve_length += str.length();

        tmp.reserve(reserve_length);    //! 预留空间

        bool is_need_insert_delimiter = false;
        for (const auto &str : str_vec) {
            if (is_need_insert_delimiter)
                tmp += delimiter;

            tmp += str;
            is_need_insert_delimiter = true;
        }
    }

    return tmp;
}

std::string StripLeft(const std::string &orig_str)
{
    size_t start_pos = orig_str.find_first_not_of(' ', 0);
    if (start_pos == std::string::npos)
        return std::string();

    return orig_str.substr(start_pos);
}

std::string StripRight(const std::string &orig_str)
{
    size_t end_pos = orig_str.find_last_not_of(' ', orig_str.size() - 1);
    if (end_pos == std::string::npos)
        return std::string();

    return orig_str.substr(0, end_pos + 1);
}

std::string Strip(const std::string &orig_str)
{
    size_t start_pos = orig_str.find_first_not_of(' ');
    if (start_pos == std::string::npos)
        return std::string();

    size_t end_pos = orig_str.find_last_not_of(' ');
    if (end_pos == std::string::npos)
        return std::string();

    return orig_str.substr(start_pos, end_pos - start_pos + 1);
}

std::string StripQuot(const std::string &orig_str)
{
    auto first_char = orig_str.front();
    auto last_char = orig_str.back();

    if (first_char == last_char && (first_char == '\'' || first_char == '\"')) {
        return orig_str.substr(1, orig_str.length() - 2);
    } else {
        return orig_str;
    }
}

std::string RawDataToHexStr(const void *data_ptr, uint16_t data_len, bool uppercase, const std::string &delimiter)
{
    if ((data_ptr == NULL) || (data_len == 0))
        return std::string();

    using namespace std;
    ostringstream oss;
    oss << hex << setfill('0');
    if (uppercase)
        oss << std::uppercase;

    const uint8_t *ptr = static_cast<const uint8_t*>(data_ptr);
    for (uint16_t i = 0; i < data_len; ++i) {
        oss << setw(2) << (int)ptr[i];
        if (i < (data_len - 1))
            oss << delimiter;
    }

    return oss.str();
}

namespace {
uint8_t hexCharToValue(char hex_char)
{
    if (('0' <= hex_char) && (hex_char <= '9'))
        return hex_char - '0';
    else if (('A' <= hex_char) && (hex_char <= 'F'))
        return hex_char - 'A' + 10;
    else if (('a' <= hex_char) && (hex_char <= 'f'))
        return hex_char - 'a' + 10;
    else
        throw NotAZaz09Exception();
}

}

size_t HexStrToRawData(const std::string &hex_str, void *out_ptr, uint16_t out_len)
{
    if ((out_ptr == NULL) || (out_len == 0))
        return 0;

    uint8_t *p_data = (uint8_t*)out_ptr;
    size_t data_len = 0;
    for (size_t i = 0; (i < out_len) && ((i * 2 + 1) < hex_str.size()); ++i) {
        char h_char = hex_str[2 * i];
        char l_char = hex_str[2 * i + 1];
        p_data[i] = (hexCharToValue(h_char) << 4) | (hexCharToValue(l_char) & 0x0f);
        ++data_len;
    }

    return data_len;
}

namespace {
void _HexStrToRawDataWithDelimiter(const std::string &hex_str, std::vector<uint8_t> &out, const std::string &delimiter)
{
    auto start_pos = hex_str.find_first_not_of(delimiter);
    while (start_pos != std::string::npos) {
        auto end_pos = hex_str.find_first_of(delimiter, start_pos);
        if (end_pos == std::string::npos)
            end_pos = hex_str.size();

        size_t len = end_pos - start_pos;
        uint8_t value = 0;
        if (len == 1) {
            value = hexCharToValue(hex_str.at(start_pos));
        } else if (len == 2) {
            value = hexCharToValue(hex_str.at(start_pos)) << 4;
            value |= hexCharToValue(hex_str.at(start_pos + 1));
        } else
            throw MoreThan2CharException();

        out.push_back(value);
        start_pos = hex_str.find_first_not_of(delimiter, end_pos);
    }
}

void _HexStrToRawDataWithoutDelimiter(const std::string &hex_str, std::vector<uint8_t> &out)
{
    auto start_pos = hex_str.find_first_not_of(" \t");
    auto end_pos = hex_str.find_last_not_of(" \t") + 1;
    for (size_t i = 0; ((i * 2) < (end_pos - start_pos)); ++i) {
        char h_char = hex_str.at(start_pos + 2 * i);
        char l_char = hex_str.at(start_pos + 2 * i + 1);
        uint8_t value = (hexCharToValue(h_char) << 4) | (hexCharToValue(l_char) & 0x0f);
        out.push_back(value);
    }
}
}

size_t HexStrToRawData(const std::string &hex_str, std::vector<uint8_t> &out, const std::string &delimiter)
{
    out.clear();

    if (delimiter.empty())
        _HexStrToRawDataWithoutDelimiter(hex_str, out);
    else
        _HexStrToRawDataWithDelimiter(hex_str, out, delimiter);

    return out.size();
}

void Replace(std::string &target_str, const std::string &pattern_str, const std::string &replace_str,
             std::string::size_type start, std::string::size_type count)
{
    if (target_str.empty() || start > target_str.size())
        return;

    if (count == 0)
        count = UINT32_MAX;

    std::string::size_type pos = start;
    std::string::size_type pattern_len = pattern_str.size();
    std::string::size_type replace_str_len = replace_str.size();

    while (count > 0 && (pos = target_str.find(pattern_str, pos)) != std::string::npos) {
        target_str.replace(pos, pattern_len, replace_str);
        pos += replace_str_len;
        --count;
    }
}

std::string ToUpper(const std::string &origin_str)
{
  std::string target_str;
  target_str.reserve(origin_str.size());
  std::back_insert_iterator<std::string>  back_insert_iter(target_str);
  std::transform(origin_str.begin(), origin_str.end(), back_insert_iter, ::toupper);
  return target_str;
}

std::string ToLower(const std::string &origin_str)
{
  std::string target_str;
  target_str.reserve(origin_str.size());
  std::back_insert_iterator<std::string>  back_insert_iter(target_str);
  std::transform(origin_str.begin(), origin_str.end(), back_insert_iter, ::tolower);
  return target_str;
}

bool IsStartWith(const std::string &origin_str, const std::string &text)
{
    if (origin_str.length() < text.length())
        return false;

    return origin_str.find(text) == 0;
}

bool IsEndWith(const std::string &origin_str, const std::string &text)
{
    if (origin_str.length() < text.length())
        return false;

    return origin_str.find(text, (origin_str.length() - text.length())) != std::string::npos;
}

}
}
}
