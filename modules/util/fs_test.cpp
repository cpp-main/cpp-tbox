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
#include "fs.h"

const char *test_filename = "./fs_test.txt";
const char *test_filename_cannot_access = "./xsdf/fs_test.txt";

using namespace tbox;
using namespace tbox::util::fs;

//! 测试文件的读、写、删除、是否存在操作，成功的用例
TEST(fs, TextFileReadWrite_Ok) {
    std::string text_tobe_read;
    const std::string text_tobe_write1 = "hello, this is a test.";
    const std::string text_tobe_write2 = "another text, yes, for test.\n  ";

    ::unlink(test_filename);  //! 先删一次

    EXPECT_FALSE(IsFileExist(test_filename));   //! 文件不应该存在

    EXPECT_TRUE(WriteStringToTextFile(test_filename, text_tobe_write1)); //! 写入数据
    EXPECT_TRUE(IsFileExist(test_filename));    //! 文件应该存在

    EXPECT_TRUE(ReadStringFromTextFile(test_filename, text_tobe_read)); //! 将文件读取出来
    EXPECT_EQ(text_tobe_write1, text_tobe_read); //! 对比写入与读出的数据是否一致

    //! 覆盖写入
    EXPECT_TRUE(WriteStringToTextFile(test_filename, text_tobe_write2)); //! 再次写入数据
    EXPECT_TRUE(ReadStringFromTextFile(test_filename, text_tobe_read)); //! 将文件读取出来
    EXPECT_EQ(text_tobe_write2, text_tobe_read); //! 对比写入与读出的数据是否一致

    EXPECT_TRUE(RemoveFile(test_filename));
    EXPECT_FALSE(IsFileExist(test_filename));
}

TEST(fs, TextFileReadAppend_Ok) {
    std::string text_tobe_read;
    const std::string text_tobe_write1 = "hello, this is a test.";
    const std::string text_tobe_write2 = "another text, yes, for test.\n  ";

    ::unlink(test_filename);  //! 先删一次
    EXPECT_FALSE(IsFileExist(test_filename));   //! 文件不应该存在

    EXPECT_TRUE(WriteStringToTextFile(test_filename, text_tobe_write1)); //! 写入数据
    EXPECT_TRUE(AppendStringToTextFile(test_filename, text_tobe_write2)); //! 写入数据
    EXPECT_TRUE(IsFileExist(test_filename));    //! 文件应该存在

    EXPECT_TRUE(ReadStringFromTextFile(test_filename, text_tobe_read)); //! 将文件读取出来
    EXPECT_EQ(text_tobe_write1 + text_tobe_write2, text_tobe_read); //! 对比写入与读出的数据是否一致

    RemoveFile(test_filename);
}

TEST(fs, TextFileReadEachLine) {
    ::unlink(test_filename);  //! 先删一次
    const std::string text_tobe_write = "first line\nsecond line\nthird line";
    EXPECT_TRUE(WriteStringToTextFile(test_filename, text_tobe_write)); //! 写入数据
    std::vector<std::string> str_vec;
    bool ret = ReadEachLineFromTextFile(test_filename,
        [&](const std::string &line) { str_vec.push_back(line); }
    );
    EXPECT_TRUE(ret);
    ASSERT_EQ(str_vec.size(), 3u);
    EXPECT_EQ(str_vec[0], "first line");
    EXPECT_EQ(str_vec[1], "second line");
    EXPECT_EQ(str_vec[2], "third line");
}

TEST(fs, FileFail) {
    EXPECT_FALSE(WriteStringToTextFile(test_filename_cannot_access, "Hello"));
    std::string text_tobe_read;
    EXPECT_FALSE(ReadStringFromTextFile(test_filename_cannot_access, text_tobe_read));

    EXPECT_FALSE(RemoveFile(test_filename_cannot_access));
}

TEST(fs, BinaryFileReadWrite_Ok) {
    std::string binary_tobe_read;
    std::string binary_tobe_write1 = "hello, this is a test.";
    std::string binary_tobe_write2 = "another binary, yes, for test.\n  ";
    //! 插入\0，模拟二进制数据中存在0数据的情况
    binary_tobe_write1[1] = '\0';
    binary_tobe_write2[1] = '\0';

    ::unlink(test_filename);  //! 先删一次

    EXPECT_FALSE(IsFileExist(test_filename));   //! 文件不应该存在

    EXPECT_TRUE(WriteBinaryToFile(test_filename, binary_tobe_write1)); //! 写入数据
    EXPECT_TRUE(IsFileExist(test_filename));    //! 文件应该存在

    EXPECT_TRUE(ReadBinaryFromFile(test_filename, binary_tobe_read)); //! 将文件读取出来
    EXPECT_EQ(binary_tobe_write1, binary_tobe_read); //! 对比写入与读出的数据是否一致

    //! 覆盖写入
    EXPECT_TRUE(WriteBinaryToFile(test_filename, binary_tobe_write2)); //! 再次写入数据
    EXPECT_TRUE(ReadBinaryFromFile(test_filename, binary_tobe_read)); //! 将文件读取出来
    EXPECT_EQ(binary_tobe_write2, binary_tobe_read); //! 对比写入与读出的数据是否一致

    EXPECT_TRUE(RemoveFile(test_filename));
    EXPECT_FALSE(IsFileExist(test_filename));
}

TEST(fs, IsDirectoryExist) {
    EXPECT_FALSE(IsDirectoryExist("should/not/exist/directory"));
    EXPECT_TRUE(IsDirectoryExist("/etc/"));
}

TEST(fs, MakeDirectory) {
    int ret = 0;

    //! 绝对路径
    EXPECT_TRUE(MakeDirectory("/tmp/fs_test_dir"));
    EXPECT_TRUE(IsDirectoryExist("/tmp/fs_test_dir"));
    ret = system("rm -rf /tmp/fs_test_dir");

    //! 相对路径
    EXPECT_TRUE(MakeDirectory("1"));
    EXPECT_TRUE(IsDirectoryExist("1"));
    ret = system("rm -rf 1");

    //! 相对路径
    EXPECT_TRUE(MakeDirectory("./a/b/c"));
    EXPECT_TRUE(IsDirectoryExist("./a/b/c"));

    //! 重复的'/'
    EXPECT_TRUE(MakeDirectory("./a///d/"));
    EXPECT_TRUE(IsDirectoryExist("a/d"));

    //! 权限不够
    ret = system("chmod 440 ./a/d");  //! 将 ./a/d 设置为 440
    EXPECT_FALSE(MakeDirectory("./a/d/e")); //! 创建失败，因为权限不够
    ret = system("rm -rf ./a");

    //! 空路径不能创建
    EXPECT_FALSE(MakeDirectory(""));

    //! 已存在对应的其它文件
    ret = system("mkdir 1; touch 1/2");
    EXPECT_FALSE(MakeDirectory("1/2/3"));   //! 不成功，因为1/2是文件，不是目录
    ret = system("rm -rf ./1");

    //! 重复创建
    EXPECT_TRUE(MakeDirectory("a/b/c"));
    EXPECT_TRUE(MakeDirectory("a/b/c"));
    ret = system("rm -rf ./a");

    ret = system("mkdir a b; cd a; ln -s ../b .;");
    EXPECT_TRUE(MakeDirectory("a/b/c"));
    EXPECT_TRUE(IsFileExist("a/b/c"));
    ret = system("rm -rf a b");
    (void)ret;
}

TEST(fs, Dirname) {
    EXPECT_EQ(Dirname("a"), ".");
    EXPECT_EQ(Dirname("a/"), "a");
    EXPECT_EQ(Dirname("a/b"), "a");
    EXPECT_EQ(Dirname("a/b/"), "a/b");
    EXPECT_EQ(Dirname(" a/b "), "a");
    EXPECT_EQ(Dirname(""), ".");
    EXPECT_EQ(Dirname(" a "), ".");
    EXPECT_EQ(Dirname("/"), "/");
    EXPECT_EQ(Dirname("/a"), "/");
    EXPECT_EQ(Dirname("/a/"), "/a");
}

TEST(fs, Basename) {
    EXPECT_EQ(Basename("./a/b/c"), "c");
    EXPECT_EQ(Basename("abcdef"), "abcdef");
    EXPECT_EQ(Basename(""), "");
}

