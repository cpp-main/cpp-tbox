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
 * ParseAs() 系列函数用于将字串解析成对应类型的值，如bool, int, double 等
 * 返回true，表示解析成功，值将存在第二个引用参数里
 * 返回false，表示解析失败，第二个引用参数将不受影响
 */
#ifndef TBOX_UTIL_PARSE_AS_H_20250626
#define TBOX_UTIL_PARSE_AS_H_20250626

#include <string>

namespace tbox {
namespace util {

bool ParseAs(const std::string &text, bool &value); //! 解析bool值

bool ParseAs(const std::string &text, int &value, int base = 10);  //! 解析int值
bool ParseAs(const std::string &text, long &value, int base = 10);  //! 解析long值
bool ParseAs(const std::string &text, long long &value, int base = 10);  //! 解析long long值
bool ParseAs(const std::string &text, unsigned int &value, int base = 10);  //! 解析unsigned int值
bool ParseAs(const std::string &text, unsigned long &value, int base = 10);  //! 解析unsigned long值
bool ParseAs(const std::string &text, unsigned long long &value, int base = 10);  //! 解析unsigned long long值

bool ParseAs(const std::string &text, float &value);   //! 解析double值
bool ParseAs(const std::string &text, double &value);  //! 解析double值

}
}

#endif //TBOX_UTIL_PARSE_AS_H_20250626
