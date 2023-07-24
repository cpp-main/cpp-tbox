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
#include <gtest/gtest.h>
#include "md5.h"
#include <tbox/util/string.h>

namespace tbox {
namespace crypto {

//! 获取数据的MD5值，以string返回
std::string Calculate(const std::string &plain_text)
{
    MD5 md5;
    uint8_t md5_digest[16];

    md5.update(plain_text.data(), plain_text.length());
    md5.finish(md5_digest);

    return tbox::util::string::RawDataToHexStr(md5_digest, 16, false, "");
}

TEST(MD5, EmptyString) {
    const char *str = "";
    EXPECT_EQ(Calculate(str), "d41d8cd98f00b204e9800998ecf8427e");
}

TEST(MD5, NormalString_1) {
    const char *str = "this is cpp-tbox, welcome.";
    EXPECT_EQ(Calculate(str), "ec10b76be2635a050e83a760c4b179b6");
}

TEST(MD5, NormalString_2) {
    const char *str = "cpp-tbox, C++ Treasure Box, is an event-based service application development library.";
    EXPECT_EQ(Calculate(str), "7a7f7121eeac412e0a286f011c46abf1");
}

TEST(MD5, UpdateTwice) {
    const char *str1 = "cpp-tbox, C++ Treasure Box,";
    const char *str2 = " is an event-based service application development library.";

    MD5 md5;
    md5.update(str1, strlen(str1));
    md5.update(str2, strlen(str2));

    uint8_t md5_digest[16];
    md5.finish(md5_digest);

    auto result = tbox::util::string::RawDataToHexStr(md5_digest, 16, false, "");
    EXPECT_EQ(result, "7a7f7121eeac412e0a286f011c46abf1");
}


}
}
