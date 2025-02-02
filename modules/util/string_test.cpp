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
#include "string.h"

using namespace tbox;
using namespace tbox::util::string;

TEST(string, Split) {
    std::vector<std::string> str_vec;
    ASSERT_EQ(Split("A::BB:C::DD ", "::", str_vec), 3u);
    EXPECT_EQ(str_vec[0], std::string("A"));
    EXPECT_EQ(str_vec[1], std::string("BB:C"));
    EXPECT_EQ(str_vec[2], std::string("DD "));

    str_vec.clear();
    ASSERT_EQ(Split("A::BB:C::DD ", ":::", str_vec), 1u);
    EXPECT_EQ(str_vec[0], std::string("A::BB:C::DD "));

    str_vec.clear();
    ASSERT_EQ(Split("A::BB:C::DD ", ":", str_vec), 6u);
    EXPECT_EQ(str_vec[0], std::string("A"));
    EXPECT_EQ(str_vec[1], std::string(""));
    EXPECT_EQ(str_vec[2], std::string("BB"));
    EXPECT_EQ(str_vec[3], std::string("C"));
    EXPECT_EQ(str_vec[4], std::string(""));
    EXPECT_EQ(str_vec[5], std::string("DD "));
}

TEST(string, SplitBySpace) {
    std::vector<std::string> str_vec;
    ASSERT_EQ(SplitBySpace(" aa bb \t  cc  d ", str_vec), 4u);
    EXPECT_EQ(str_vec[0], "aa");
    EXPECT_EQ(str_vec[1], "bb");
    EXPECT_EQ(str_vec[2], "cc");
    EXPECT_EQ(str_vec[3], "d");

    str_vec.clear();
    ASSERT_EQ(SplitBySpace("aa b", str_vec), 2u);
    EXPECT_EQ(str_vec[0], "aa");
    EXPECT_EQ(str_vec[1], "b");

}

TEST(string, JoinStrVecNormal) {
    std::vector<std::string> str_vec = {"aa", "bb", "cc"};
    EXPECT_EQ(Join(str_vec, ":"), "aa:bb:cc");
}

TEST(string, JoinStrVecNoDelimeter) {
    std::vector<std::string> str_vec = {"aa", "bb"};
    EXPECT_EQ(Join(str_vec), "aabb");
}

TEST(string, JoinEmptyStrVec_1) {
    std::vector<std::string> str_vec;
    EXPECT_EQ(Join(str_vec, ":"), "");
}

TEST(string, JoinEmptyStrVec_2) {
    std::vector<std::string> str_vec = {"", "", ""};
    EXPECT_EQ(Join(str_vec, ":"), "::");
}

TEST(string, StripLeft) {
    EXPECT_EQ(StripLeft(" A "), "A ");
}

TEST(string, StripRight) {
    EXPECT_EQ(StripRight(" A "), " A");
}

TEST(string, Strip) {
    EXPECT_EQ(Strip(" A "), "A");
    EXPECT_EQ(Strip("  "), "");
}

TEST(string, StripQuot) {
    EXPECT_EQ(StripQuot(R"("A")"), "A");
    EXPECT_EQ(StripQuot(R"('A')"), "A");
    EXPECT_EQ(StripQuot(R"(A)"), "A");
    EXPECT_EQ(StripQuot(R"("A)"), R"("A)");
    EXPECT_EQ(StripQuot(R"(A")"), R"(A")");
    EXPECT_EQ(StripQuot(R"('A)"), R"('A)");
    EXPECT_EQ(StripQuot(R"(A')"), R"(A')");
}

TEST(string, RawDataToHexStr) {
    uint8_t tmp[] = { 0x0e, 0x00, 0xa8 };
    EXPECT_EQ(RawDataToHexStr(tmp, sizeof(tmp), false, ""), "0e00a8");
    EXPECT_EQ(RawDataToHexStr(tmp, sizeof(tmp), false, " "), "0e 00 a8");
    EXPECT_EQ(RawDataToHexStr(tmp, sizeof(tmp), true, ""), "0E00A8");
    EXPECT_EQ(RawDataToHexStr(tmp, sizeof(tmp), true, " "), "0E 00 A8");
}

TEST(string, HexStrToRawDataFixSizeBuffer) {
    uint8_t tmp[100] = { 0 };
    EXPECT_EQ(HexStrToRawData("0123456789aBcDEf", tmp, 16), 8u);
    EXPECT_EQ(tmp[0], 0x01);
    EXPECT_EQ(tmp[1], 0x23);
    EXPECT_EQ(tmp[2], 0x45);
    EXPECT_EQ(tmp[3], 0x67);
    EXPECT_EQ(tmp[4], 0x89);
    EXPECT_EQ(tmp[5], 0xab);
    EXPECT_EQ(tmp[6], 0xcd);
    EXPECT_EQ(tmp[7], 0xef);
    EXPECT_EQ(tmp[8], 0x00);
    EXPECT_EQ(tmp[9], 0x00);

    //! 验证提供的输出缓冲不够的情况
    memset(tmp, 0, sizeof(tmp));
    EXPECT_EQ(HexStrToRawData("0123456789aBcDEf", tmp, 3), 3u);
    EXPECT_EQ(tmp[0], 0x01);
    EXPECT_EQ(tmp[1], 0x23);
    EXPECT_EQ(tmp[2], 0x45);
    EXPECT_EQ(tmp[3], 0x00);
    EXPECT_EQ(tmp[4], 0x00);

    //! 验证提供的HEX字串不全的情况
    memset(tmp, 0, sizeof(tmp));
    EXPECT_EQ(HexStrToRawData("01234", tmp, sizeof(tmp)), 2u);
    EXPECT_EQ(tmp[0], 0x01);
    EXPECT_EQ(tmp[1], 0x23);
    EXPECT_EQ(tmp[2], 0x00);
    EXPECT_EQ(tmp[3], 0x00);
}

TEST(string, HexStrToRawDataVector1) {
    std::vector<uint8_t> out;
    HexStrToRawData("01 2f Ab 67", out, " \t");
    EXPECT_EQ(out.size(), 4u);
    EXPECT_EQ(out[0], 0x01);
    EXPECT_EQ(out[1], 0x2f);
    EXPECT_EQ(out[2], 0xAb);
    EXPECT_EQ(out[3], 0x67);
}

TEST(string, HexStrToRawDataVector1_1) {
    std::vector<uint8_t> out;
    HexStrToRawData("01:2f:Ab:67", out, ":");
    EXPECT_EQ(out.size(), 4u);
    EXPECT_EQ(out[0], 0x01);
    EXPECT_EQ(out[1], 0x2f);
    EXPECT_EQ(out[2], 0xAb);
    EXPECT_EQ(out[3], 0x67);
}

TEST(string, HexStrToRawDataVector1_2) {
    std::vector<uint8_t> out;
    HexStrToRawData(" : 01 :2f : Ab: 67 :", out, ": ");
    EXPECT_EQ(out.size(), 4u);
    EXPECT_EQ(out[0], 0x01);
    EXPECT_EQ(out[1], 0x2f);
    EXPECT_EQ(out[2], 0xAb);
    EXPECT_EQ(out[3], 0x67);
}

TEST(string, HexStrToRawDataVector2) {
    std::vector<uint8_t> out;
    HexStrToRawData("\t 01 \t23 \t 45\t67\t ", out, "\t ");
    EXPECT_EQ(out.size(), 4u);
    EXPECT_EQ(out[0], 0x01);
    EXPECT_EQ(out[1], 0x23);
    EXPECT_EQ(out[2], 0x45);
    EXPECT_EQ(out[3], 0x67);
}

TEST(string, HexStrToRawDataVector3) {
    std::vector<uint8_t> out;
    HexStrToRawData("1 2 3 4", out, " ");
    EXPECT_EQ(out.size(), 4u);
    EXPECT_EQ(out[0], 0x01);
    EXPECT_EQ(out[1], 0x02);
    EXPECT_EQ(out[2], 0x03);
    EXPECT_EQ(out[3], 0x04);
}

TEST(string, HexStrToRawDataVector4) {
    std::vector<uint8_t> out;
    HexStrToRawData("   1     2 ", out, " ");
    EXPECT_EQ(out.size(), 2u);
    EXPECT_EQ(out[0], 0x01);
    EXPECT_EQ(out[1], 0x02);
}

TEST(string, HexStrToRawDataVector5) {
    std::vector<uint8_t> out;
    EXPECT_THROW(HexStrToRawData("ZY", out), NotAZaz09Exception);
}

TEST(string, HexStrToRawDataVector6) {
    std::vector<uint8_t> out;
    EXPECT_THROW(HexStrToRawData(" 123  ", out, " "), MoreThan2CharException);
}

TEST(string, HexStrToRawDataVector7) {
    std::vector<uint8_t> out;
    HexStrToRawData("    ", out, " ");
    EXPECT_EQ(out.size(), 0u);
}

TEST(string, HexStrToRawDataVector8) {
    std::vector<uint8_t> out;
    EXPECT_THROW(HexStrToRawData(" __zs a", out, " "), MoreThan2CharException);
}

TEST(string, HexStrToRawDataVector9) {
    std::vector<uint8_t> out;
    HexStrToRawData("123456", out);
    EXPECT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], 0x12);
    EXPECT_EQ(out[1], 0x34);
    EXPECT_EQ(out[2], 0x56);
}

TEST(string, HexStrToRawDataVector10) {
    std::vector<uint8_t> out;
    HexStrToRawData("  123456 ", out);
    EXPECT_EQ(out.size(), 3u);
    EXPECT_EQ(out[0], 0x12);
    EXPECT_EQ(out[1], 0x34);
    EXPECT_EQ(out[2], 0x56);
}

TEST(string, HexStrToRawDataVector11) {
    std::vector<uint8_t> out;
    EXPECT_THROW(HexStrToRawData("12 3456", out), NotAZaz09Exception);
}

TEST(string, Replace) {
    if (true) { //! 全替换
        std::string t("aa bbaa aab aaaa aab");
        std::string r;
        Replace(t, "aa", "cc");
        EXPECT_STREQ(t.c_str(), "cc bbcc ccb cccc ccb");
    }

    if (true) { //! 从某位置替换
        std::string t("aa bbaa aab aaaa aab");
        std::string r;
        Replace(t, "aa", "cc", 6);
        EXPECT_STREQ(t.c_str(), "aa bbaa ccb cccc ccb");
    }

    if (true) { //! 指定位置与替换次数
        std::string t("aa bbaa aab aaaa aab");
        std::string r;
        Replace(t, "aa", "cc", 6, 2);
        EXPECT_STREQ(t.c_str(), "aa bbaa ccb ccaa aab");
    }

    if (true) { //! 起始位置超出
        std::string t("aa bbaa aab aaaa aab");
        std::string r;
        Replace(t, "aa", "cc", t.size());
        EXPECT_STREQ(t.c_str(), "aa bbaa aab aaaa aab");
    }
}

TEST(string, ToUpper) {
  EXPECT_EQ(ToUpper("Abc:?x#Y$z "), "ABC:?X#Y$Z ");
  EXPECT_EQ(ToUpper(""), "");
}

TEST(string, ToLower) {
  EXPECT_EQ(ToLower("Abc:?x#Y$z "), "abc:?x#y$z ");
  EXPECT_EQ(ToLower(""), "");
}

TEST(string, IsStartWith) {
  EXPECT_TRUE(IsStartWith("abc.123", "abc"));
  EXPECT_FALSE(IsStartWith("abc.123", "12"));
  EXPECT_FALSE(IsStartWith("abc", "abcd"));
}

TEST(string, IsEndWith) {
  EXPECT_TRUE(IsEndWith("abc.123", "123"));
  EXPECT_FALSE(IsEndWith("abc.123", "bc"));
  EXPECT_FALSE(IsEndWith("abc", "abcd"));
}

