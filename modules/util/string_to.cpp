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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */

#include "string_to.h"

#include <tbox/base/defines.h>
#include "string.h"

namespace tbox {
namespace util {

bool StringTo(const std::string &text, bool &value)
{
    const char *text_tbl[] = {
        "YES", "NO",
        "YEP", "NOPE",
        "Y", "N",
        "TRUE", "FALSE",
        "T", "F",
        "ON", "OFF",
    };

    auto upper_text = string::ToUpper(text);
    for (size_t i = 0; i < NUMBER_OF_ARRAY(text_tbl); ++i) {
        if (upper_text == text_tbl[i]) {
            value = ((i & 1) == 0);
            return true;
        }
    }

    return false;
}

#define TO_NUMBER(func) \
    try { \
        size_t size = 0; \
        auto tmp = func(text, &size); \
        if (size == text.size()) { \
            value = tmp; \
            return true; \
        } \
    } catch (...) { } \
    return false

#define TO_NUMBER_WITH_BASE(func) \
    try { \
        size_t size = 0; \
        auto tmp = func(text, &size, base); \
        if (size == text.size()) { \
            value = tmp; \
            return true; \
        } \
    } catch (...) { } \
    return false

bool StringTo(const std::string &text, int &value, int base)
{
    TO_NUMBER_WITH_BASE(std::stoi);
}

bool StringTo(const std::string &text, long &value, int base)
{
    TO_NUMBER_WITH_BASE(std::stol);
}

bool StringTo(const std::string &text, long long &value, int base)
{
    TO_NUMBER_WITH_BASE(std::stoll);
}

bool StringTo(const std::string &text, unsigned int &value, int base)
{
    TO_NUMBER_WITH_BASE(std::stoul);
}

bool StringTo(const std::string &text, unsigned long &value, int base)
{
    TO_NUMBER_WITH_BASE(std::stoul);
}

bool StringTo(const std::string &text, unsigned long long &value, int base)
{
    TO_NUMBER_WITH_BASE(std::stoull);
}

bool StringTo(const std::string &text, float &value)
{
    TO_NUMBER(std::stof);
}

bool StringTo(const std::string &text, double &value)
{
    TO_NUMBER(std::stod);
}

}
}
