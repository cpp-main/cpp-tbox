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
#include "json.h"

#include <fstream>

#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>

namespace tbox {
namespace util {
namespace json {

bool Get(const Json &js, bool &field_value)
{
    if (!js.is_boolean())
        return false;
    field_value = js.get<bool>();
    return true;
}

bool Get(const Json &js, unsigned int &field_value) {
  if (!js.is_number_unsigned())
      return false;
  field_value = js.get<unsigned int>();
  return true;
}

bool Get(const Json &js, double &field_value)
{
    if (!js.is_number())
        return false;
    field_value = js.get<double>();
    return true;
}

bool Get(const Json &js, std::string &field_value)
{
    if (!js.is_string())
        return false;
    field_value = js.get<std::string>();
    return true;
}

bool Get(const Json &js,int &field_value)
{
    if (!js.is_number_integer())
        return false;
    field_value = js.get<int>();
    return true;
}

bool GetField(const Json &js, const std::string &field_name, bool &field_value)
{
    if (!js.contains(field_name))
        return false;
    return Get(js.at(field_name), field_value);
}

bool GetField(const Json &js, const std::string &field_name, unsigned int &field_value)
{
    if (!js.contains(field_name))
        return false;
    return Get(js.at(field_name), field_value);
}

bool GetField(const Json &js, const std::string &field_name, int &field_value)
{
    if (!js.contains(field_name))
        return false;
    return Get(js.at(field_name), field_value);
}

bool GetField(const Json &js, const std::string &field_name, double &field_value)
{
    if (!js.contains(field_name))
        return false;
    return Get(js.at(field_name), field_value);
}

bool GetField(const Json &js, const std::string &field_name, std::string &field_value)
{
    if (!js.contains(field_name))
        return false;
    return Get(js.at(field_name), field_value);
}

bool HasObjectField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_object();
}

bool HasArrayField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_array();
}

bool HasBooleanField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_boolean();
}

bool HasNumberField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_number();
}

bool HasFloatField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_number_float();
}

bool HasIntegerField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_number_integer();
}

bool HasUnsignedField(const Json &js, const std::string &field_name)
{
    if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_number_unsigned();
}

bool HasStringField(const Json &js, const std::string &field_name)
{
     if (!js.contains(field_name))
        return false;
    auto &js_field = js.at(field_name);
    return js_field.is_string();
}

Json Load(const std::string &filename)
{
    Json js;
    std::ifstream input_json_file(filename);
    if (input_json_file.is_open()) {
        try {
            input_json_file >> js;
        } catch (const std::exception &e) {
            throw ParseJsonFileError(filename, e.what());
        }
    } else {
        throw OpenFileError(filename);
    }

    return js;
}

/**
 * 通过数[],{},"的方式找JSON字串的结束位置
 */
int FindEndPos(const char *str_ptr, size_t str_len)
{
    TBOX_ASSERT(str_ptr != nullptr);

    bool is_started = false;//! 是否已开始，防止开头是空格或TAB
    int braces_level = 0;   //! 花括号的层级
    int square_level = 0;   //! 方括号的层级
    bool in_string = false; //! 是否在字串中

    for (size_t i = 0; i < str_len; ++i) {
        char ch = str_ptr[i];
        if (!is_started && ::isgraph(ch))
            is_started = true;

        if (ch == '"') {
            if (in_string) {
                //! 要防止 \" 与 \\" \\\\\" 这些转义符的情况
                //! 如果 " 前面是连续的 \ 那就对应数 \ 的个数。如果是单数，则"失效
                in_string = false;
                for (size_t j = (i - 1); j != 0 && str_ptr[j] == '\\'; --j)
                    in_string = !in_string;
            } else {
                in_string = true;
            }
        } else {
            if (in_string)  //! 如果在字串里，就直接略过
                continue;

            switch (ch) {
                case '[': ++square_level; break;
                case ']': --square_level; break;
                case '{': ++braces_level; break;
                case '}': --braces_level; break;
            }
        }

        if (braces_level == 0 &&
            square_level == 0 &&
            !in_string &&
            is_started) {
            return i + 1;
        }

        if (braces_level < 0 || square_level < 0)
            return -1;
    }

    return 0;
}

}
}
}
