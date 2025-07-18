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
#include <tbox/base/log.h>
#include <tbox/base/json.hpp>

#include "string.h"

namespace tbox {
namespace util {

bool StringTo(const std::string &text, bool &value)
{
    const char *text_tbl[] = {
        "TRUE", "FALSE",
        "YES", "NO",
        "ON", "OFF",
        "1", "0",
        "ENABLE", "DISABLE",
        "Y", "N",
        "T", "F",
        "YEP", "NOPE",
        "ACTIVE", "INACTIVE",
        "POSITIVE", "NEGATIVE",
        "POS", "NEG",
        "+", "-",
    };

    auto upper_text = string::ToUpper(text);
    for (size_t i = 0; i < NUMBER_OF_ARRAY(text_tbl); ++i) {
        if (upper_text == text_tbl[i]) {
            value = ((i & 1) == 0);
            return true;
        }
    }

    LogNotice("can't convert '%s' to bool", text.c_str());
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
    } catch (...) { \
        LogNotice("can't convert '%s' to number", text.c_str()); \
    } \
    return false

#define TO_NUMBER_WITH_BASE(func) \
    try { \
        size_t size = 0; \
        auto tmp = func(text, &size, base); \
        if (size == text.size()) { \
            value = tmp; \
            return true; \
        } \
    } catch (...) { \
        LogNotice("can't convert '%s' to integer with base %d", text.c_str(), base); \
    } \
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

#if __SIZEOF_INT__ > 4
bool StringTo(const std::string &text, uint32_t &value, int base)
{
    uint64_t tmp;

    if (StringTo(text, tmp, base)) {
        if (tmp < 0x100000000lu) {
            value = static_cast<uint32_t>(tmp);
            return true;
        } else {
            LogWarn("number overflow, %u > 0xffffffff", tmp);
        }
    }

    return false;
}
#endif

bool StringTo(const std::string &text, uint16_t &value, int base)
{
    uint32_t tmp;

    if (StringTo(text, tmp, base)) {
        if (tmp < 0x10000u) {
            value = static_cast<uint16_t>(tmp);
            return true;
        } else {
            LogWarn("number overflow, %u > 0xffff", tmp);
        }
    }

    return false;
}

bool StringTo(const std::string &text, uint8_t &value, int base)
{
    uint16_t tmp;

    if (StringTo(text, tmp, base)) {
        if (tmp < 0x100u) {
            value = static_cast<uint8_t>(tmp);
            return true;
        } else {
            LogWarn("number overflow, %u > 0xff", tmp);
        }
    }

    return false;
}

bool StringTo(const std::string &text, float &value)
{
    TO_NUMBER(std::stof);
}

bool StringTo(const std::string &text, double &value)
{
    TO_NUMBER(std::stod);
}

bool StringTo(const std::string &text, std::string &value)
{
    value = text;
    return true;
}

bool StringTo(const std::string &text, Json &js_value)
{
    try {
        js_value = Json::parse(text);
        return true;

    } catch (const std::exception &e) {
        LogNotice("can't convert '%s' to json, what: %s", text.c_str(), e.what());
        return false;
    }
}

}
}
