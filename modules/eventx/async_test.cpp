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
#include "async.h"

#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/eventx/thread_pool.h>
#include <tbox/util/fs.h>

namespace tbox {
namespace eventx {
namespace {

class AsyncTest : public testing::Test {
  protected:
    event::Loop *loop_;
    eventx::ThreadPool *thread_pool_;
    Async *async_;
    std::string filename = "/tmp/async_test.txt";

  public:
    void SetUp() override {
        loop_ = event::Loop::New();
        thread_pool_ = new eventx::ThreadPool(loop_);
        thread_pool_->initialize(1, 1);
        async_ = new Async(thread_pool_);
    }

    void TearDown() override {
        thread_pool_->cleanup();
        delete async_;
        delete thread_pool_;
        delete loop_;
        util::fs::RemoveFile(filename);
    }
};

/// 测试 Async::writeFile() 函数
TEST_F(AsyncTest, WriteFile) {
    std::string wcontent = "This is AsyncTest::WriteFile";
    int errcode = -1;

    async_->writeFile(filename, wcontent, false, [&](int code) { errcode = code; });

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    ASSERT_EQ(errcode, 0);

    std::string rcontent;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(filename, rcontent));
    EXPECT_EQ(rcontent, wcontent);
}

/// 测试 Async::writeFile() 函数，不带回调
TEST_F(AsyncTest, WriteFileNoCallback) {
    std::string wcontent = "This is AsyncTest::WriteFile";

    async_->writeFile(filename, wcontent);

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    std::string rcontent;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(filename, rcontent));
    EXPECT_EQ(rcontent, wcontent);
}

/// 测试 Async::appendFile() 函数
TEST_F(AsyncTest, appendFile) {
    std::string wcontent_1 = "This is string1;";
    std::string wcontent_2 = "This is string2.";
    int errcode_1 = -1;
    int errcode_2 = -1;

    async_->appendFile(filename, wcontent_1, false, [&](int code) { errcode_1 = code; });
    async_->appendFile(filename, wcontent_2, false, [&](int code) { errcode_2 = code; });

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    ASSERT_EQ(errcode_1, 0);
    ASSERT_EQ(errcode_2, 0);

    std::string rcontent;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(filename, rcontent));
    EXPECT_EQ(rcontent, wcontent_1 + wcontent_2);
}

/// 测试 Async::appendFile() 函数，不带回调
TEST_F(AsyncTest, appendFileNoCallback) {
    std::string wcontent_1 = "This is string1;";
    std::string wcontent_2 = "This is string2.";

    async_->appendFile(filename, wcontent_1);
    async_->appendFile(filename, wcontent_2);

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    std::string rcontent;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(filename, rcontent));
    EXPECT_EQ(rcontent, wcontent_1 + wcontent_2);
}

/// 测试 Async::readFile() 函数
TEST_F(AsyncTest, ReadFile) {
    std::string wcontent = "This is AsyncTest::ReadFile";

    ASSERT_TRUE(util::fs::WriteStringToTextFile(filename, wcontent));

    int errcode = -1;
    std::string rcontent;
    async_->readFile(filename,
        [&](int code, const std::string &content) {
            errcode = code;
            rcontent = content;
        }
    );

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    ASSERT_EQ(errcode, 0);
    EXPECT_EQ(rcontent, wcontent);
}

/// 测试 Async::readFileLines() 函数
TEST_F(AsyncTest, ReadFileLines) {
    std::string wcontent = "first\nsecond\nthird";

    ASSERT_TRUE(util::fs::WriteStringToTextFile(filename, wcontent));

    int errcode = -1;
    std::vector<std::string> rcontent_vec;
    async_->readFileLines(filename,
        [&](int code, std::vector<std::string> &lines) {
            errcode = code;
            rcontent_vec = std::move(lines);
        }
    );

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    ASSERT_EQ(errcode, 0);
    EXPECT_EQ(rcontent_vec[0], "first");
    EXPECT_EQ(rcontent_vec[1], "second");
    EXPECT_EQ(rcontent_vec[2], "third");
}

/// 测试 Async::removeFile() 函数，不带回调
TEST_F(AsyncTest, removeFileNoCallback) {
    util::fs::WriteStringToTextFile(filename, "any");
    ASSERT_TRUE(util::fs::IsFileExist(filename));

    async_->removeFile(filename);

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    ASSERT_FALSE(util::fs::IsFileExist(filename));
}

TEST_F(AsyncTest, executeCmdOnly) {
    std::string wcontent = "This is AsyncTest::executeCmd";

    async_->executeCmd("echo -n '" + wcontent + "' > " + filename);

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    std::string rcontent;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(filename, rcontent));
    EXPECT_EQ(rcontent, wcontent);
}

TEST_F(AsyncTest, executeCmdThenGetResult) {
    std::string wcontent = "This is AsyncTest::executeCmd with cb";
    ASSERT_TRUE(util::fs::WriteStringToTextFile(filename, wcontent));

    int errcode = -1;
    std::string rcontent;
    async_->executeCmd("cat " + filename,
        [&](int code, const std::string &content) {
            errcode = code;
            rcontent = content;
        }
    );

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    ASSERT_EQ(errcode, 0);
    EXPECT_EQ(rcontent, wcontent);
}

}
}
}
