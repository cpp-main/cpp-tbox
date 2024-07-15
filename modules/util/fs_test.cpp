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
    const std::string text_tobe_write = "first line\r\nsecond line\nthird line";
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

TEST(fs, TextFileReadFirstLine) {
    ::unlink(test_filename);  //! 先删一次
    EXPECT_TRUE(WriteStringToTextFile(test_filename, "first line\r\n")); //! 写入数据
    std::string text;
    EXPECT_TRUE(ReadFirstLineFromTextFile(test_filename, text));
    ASSERT_EQ(text, "first line");

    ::unlink(test_filename);  //! 先删一次
    EXPECT_TRUE(WriteStringToTextFile(test_filename, "one line")); //! 写入数据
    EXPECT_TRUE(ReadFirstLineFromTextFile(test_filename, text));
    ASSERT_EQ(text, "one line");
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

TEST(fs, RenameFile) {
  auto old_name = "old_file";
  auto new_name = "new_file";
  EXPECT_TRUE(WriteStringToTextFile(old_name, ""));
  EXPECT_TRUE(Rename(old_name, new_name));
  EXPECT_TRUE(IsFileExist(new_name));
  EXPECT_FALSE(IsFileExist(old_name));
  RemoveFile(new_name);
}

TEST(fs, RenameDirectory) {
  auto old_name = "old_dir";
  auto new_name = "new_dir";
  EXPECT_TRUE(MakeDirectory(old_name));
  EXPECT_TRUE(Rename(old_name, new_name));
  EXPECT_TRUE(IsDirectoryExist(new_name));
  EXPECT_FALSE(IsFileExist(old_name));
  RemoveDirectory(old_name);
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
    EXPECT_STREQ(Basename("./a/b/c"), "c");
    EXPECT_STREQ(Basename("abcdef"), "abcdef");
    EXPECT_STREQ(Basename(""), "");

    EXPECT_EQ(Basename(std::string("./a/b/c")), "c");
    EXPECT_EQ(Basename(std::string("abcdef")), "abcdef");
    EXPECT_EQ(Basename(std::string()), "");
}

TEST(fs, RemoveDirectory) {
    //! 绝对路径测试
    int ret = 0;
    ret = ::system("rm -rf /tmp/fs_test_dir");
    ret = ::system("mkdir -p /tmp/fs_test_dir/first_dir");
    (void)ret;

    WriteStringToTextFile("/tmp/fs_test_dir/fs_test_0_1.txt", "hello, this is a test file");
    WriteStringToTextFile("/tmp/fs_test_dir/fs_test_0_2.txt", "hello, this is a test file");
    WriteStringToTextFile("/tmp/fs_test_dir/first_dir/fs_test_1_1.txt", "hello, this is a test file");
    WriteStringToTextFile("/tmp/fs_test_dir/first_dir/fs_test_1_2.txt", "hello, this is a test file");

    EXPECT_TRUE(RemoveDirectory("/tmp/fs_test_dir", true));     //! 仅删文件，不删目录
    EXPECT_TRUE(IsDirectoryExist("/tmp/fs_test_dir/first_dir/"));

    EXPECT_TRUE(RemoveDirectory("/tmp/fs_test_dir", false));    //! 全部删除
    EXPECT_FALSE(IsDirectoryExist("/tmp/fs_test_dir"));

    //! 相对路径测试，一层目录
    MakeDirectory("./a");
    WriteStringToTextFile("./a/fs_test_a_1.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/fs_test_a_2.txt", "hello, this is a test file");
    EXPECT_TRUE(RemoveDirectory("./a"));
    EXPECT_FALSE(IsDirectoryExist("./a"));

    //! 相对路径测试，多层目录
    MakeDirectory("./a/b/c");
    WriteStringToTextFile("./a/fs_test_a_1.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/fs_test_a_2.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/b/fs_test_b_1.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/b/fs_test_b_2.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/b/c/fs_test_c_1.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/b/c/fs_test_c_2.txt", "hello, this is a test file");
    EXPECT_TRUE(RemoveDirectory("./a"));
    EXPECT_FALSE(IsDirectoryExist("./a"));

    //! 重复的'/' 测试
    MakeDirectory("./a/b");
    WriteStringToTextFile("./a/fs_test_a_1.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/fs_test_a_2.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/b/fs_test_b_1.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/b/fs_test_b_2.txt", "hello, this is a test file");
    EXPECT_TRUE(RemoveDirectory("./a//b"));
    EXPECT_FALSE(IsDirectoryExist("./a/b"));
    EXPECT_TRUE(RemoveDirectory("./a/"));
    EXPECT_FALSE(IsDirectoryExist("./a"));

    //! 目录尾部多加一个 '/' 测试
    MakeDirectory("./a/b");
    WriteStringToTextFile("./a/fs_test_a_1.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/fs_test_a_2.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/b/fs_test_b_1.txt", "hello, this is a test file");
    WriteStringToTextFile("./a/b/fs_test_b_2.txt", "hello, this is a test file");
    EXPECT_TRUE(RemoveDirectory("./a/"));
    EXPECT_FALSE(IsDirectoryExist("./a"));

    // 只有目录，没有文件的删除测试
    MakeDirectory("./a/b1/c");
    MakeDirectory("./a/b2/c");
    MakeDirectory("./a/b3/c");
    EXPECT_TRUE(RemoveDirectory("./a"));
    EXPECT_FALSE(IsDirectoryExist("./a"));
}
