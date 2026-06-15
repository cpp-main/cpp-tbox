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
#include <gtest/gtest.h>
#include <cstring>

#include "sha1.h"

namespace tbox {
namespace crypto {

TEST(SHA1, EmptyString)
{
    //! SHA-1("") = da39a3ee5e6b4b0d3255bfef95601890afd80709
    uint8_t digest[20];
    SHA1::Calc("", 0, digest);

    const uint8_t expected[] = {
        0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d,
        0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90,
        0xaf, 0xd8, 0x07, 0x09
    };

    EXPECT_EQ(0, memcmp(digest, expected, 20));
}

TEST(SHA1, Abc)
{
    //! SHA-1("abc") = a9993e364706816aba3e25717850c26c9cd0d89d
    SHA1 sha1;
    sha1.update("abc", 3);
    uint8_t digest[20];
    sha1.finish(digest);

    const uint8_t expected[] = {
        0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a,
        0xba, 0x3e, 0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c,
        0x9c, 0xd0, 0xd8, 0x9d
    };

    EXPECT_EQ(0, memcmp(digest, expected, 20));
}

TEST(SHA1, UpdateTwice)
{
    //! SHA-1("abc") via two updates should equal SHA-1("abc") via one update
    SHA1 sha1_1;
    sha1_1.update("a", 1);
    sha1_1.update("bc", 2);
    uint8_t digest1[20];
    sha1_1.finish(digest1);

    uint8_t digest2[20];
    SHA1::Calc("abc", 3, digest2);

    EXPECT_EQ(0, memcmp(digest1, digest2, 20));
}

}
}
