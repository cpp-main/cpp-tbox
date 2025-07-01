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

/**
 * StringTo() 系列函数用于将字串解析成对应类型的值，如bool, int, double 等
 *
 * 返回true，表示解析成功，值将存在第二个引用参数里
 * 返回false，表示解析失败，第二个引用参数将不受影响
 */
#ifndef TBOX_UTIL_STRING_TO_H_20250626
#define TBOX_UTIL_STRING_TO_H_20250626

#include <string>
#include <tbox/base/json_fwd.h>

namespace tbox {
namespace util {

/**
 * 解析字串为bool值
 *
 * 真值: true,  yes, on,  1, enable,  y, t, yep,  active,   positive, pos, +
 * 假值: false, no,  off, 0, disable, n, f, nope, inactive, negative, neg, -
 *
 * 大小写不敏感
 */
bool StringTo(const std::string &text, bool &value);

bool StringTo(const std::string &text, int &value, int base = 10);  //! 解析int值
bool StringTo(const std::string &text, long &value, int base = 10);  //! 解析long值
bool StringTo(const std::string &text, long long &value, int base = 10);  //! 解析long long值
bool StringTo(const std::string &text, unsigned int &value, int base = 10);  //! 解析unsigned int值
bool StringTo(const std::string &text, unsigned long &value, int base = 10);  //! 解析unsigned long值
bool StringTo(const std::string &text, unsigned long long &value, int base = 10);  //! 解析unsigned long long值

#if __SIZEOF_INT__ > 4
bool StringTo(const std::string &text, uint32_t &value, int base = 10);  //! 解析uint32值
#endif
bool StringTo(const std::string &text, uint16_t &value, int base = 10);  //! 解析uint16值
bool StringTo(const std::string &text, uint8_t &value, int base = 10);   //! 解析uint8值

bool StringTo(const std::string &text, float &value);   //! 解析float值
bool StringTo(const std::string &text, double &value);  //! 解析double值

bool StringTo(const std::string &text, std::string &value); //! 等效于 value = text，仅用于风格一致性

bool StringTo(const std::string &text, Json &value);    //! 解析成JSON对象

}
}

#endif //TBOX_UTIL_STRING_TO_H_20250626
