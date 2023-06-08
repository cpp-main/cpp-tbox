#include "system.h"

#include <gtest/gtest.h>
#include <tbox/event/loop.h>
#include <tbox/eventx/thread_pool.h>
#include <tbox/util/fs.h>

namespace tbox {
namespace system {
namespace {

class SystemTest : public testing::Test {
  protected:
    event::Loop *loop_;
    eventx::ThreadPool *thread_pool_;
    System *sys_;
    std::string filename = "/tmp/system_test.txt";

  public:
    void SetUp() override {
        loop_ = event::Loop::New();
        thread_pool_ = new eventx::ThreadPool(loop_);
        thread_pool_->initialize(1, 1);
        sys_ = new System(thread_pool_);
    }

    void TearDown() override {
        thread_pool_->cleanup();
        delete sys_;
        delete thread_pool_;
        delete loop_;
        util::fs::RemoveFile(filename);
    }
};

/// 测试 System::writeFile() 函数
TEST_F(SystemTest, WriteFile) {
    std::string wcontent = "This is SystemTest::WriteFile";
    int errcode = -1;

    sys_->writeFile(filename, wcontent, false, [&](int code) { errcode = code; });

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    ASSERT_EQ(errcode, 0);

    std::string rcontent;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(filename, rcontent));
    EXPECT_EQ(rcontent, wcontent);
}

/// 测试 System::writeFile() 函数，不带回调
TEST_F(SystemTest, WriteFileNoCallback) {
    std::string wcontent = "This is SystemTest::WriteFile";

    sys_->writeFile(filename, wcontent);

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    std::string rcontent;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(filename, rcontent));
    EXPECT_EQ(rcontent, wcontent);
}

/// 测试 System::appendFile() 函数
TEST_F(SystemTest, appendFile) {
    std::string wcontent_1 = "This is string1;";
    std::string wcontent_2 = "This is string2.";
    int errcode_1 = -1;
    int errcode_2 = -1;

    sys_->appendFile(filename, wcontent_1, false, [&](int code) { errcode_1 = code; });
    sys_->appendFile(filename, wcontent_2, false, [&](int code) { errcode_2 = code; });

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    ASSERT_EQ(errcode_1, 0);
    ASSERT_EQ(errcode_2, 0);

    std::string rcontent;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(filename, rcontent));
    EXPECT_EQ(rcontent, wcontent_1 + wcontent_2);
}

/// 测试 System::appendFile() 函数，不带回调
TEST_F(SystemTest, appendFileNoCallback) {
    std::string wcontent_1 = "This is string1;";
    std::string wcontent_2 = "This is string2.";

    sys_->appendFile(filename, wcontent_1);
    sys_->appendFile(filename, wcontent_2);

    loop_->exitLoop(std::chrono::milliseconds(100));
    loop_->runLoop();

    std::string rcontent;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(filename, rcontent));
    EXPECT_EQ(rcontent, wcontent_1 + wcontent_2);
}


/// 测试 System::readFile() 函数
TEST_F(SystemTest, ReadFile) {
    std::string wcontent = "This is SystemTest::ReadFile";

    ASSERT_TRUE(util::fs::WriteStringToTextFile(filename, wcontent));

    int errcode = -1;
    std::string rcontent;
    sys_->readFile(filename,
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

/// 测试 System::readFileLines() 函数
TEST_F(SystemTest, ReadFileLines) {
    std::string wcontent = "first\nsecond\nthird";

    ASSERT_TRUE(util::fs::WriteStringToTextFile(filename, wcontent));

    int errcode = -1;
    std::vector<std::string> rcontent_vec;
    sys_->readFileLines(filename,
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

}
}
}
