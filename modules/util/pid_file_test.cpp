#include <thread>
#include <gtest/gtest.h>
#include "pid_file.h"

using namespace tbox::util;

TEST(PidFile, base)
{
    const char *pid_file = "/tmp/test.pid";
    ::unlink(pid_file);
    {
        PidFile pid;
        ASSERT_TRUE(pid.lock(pid_file));

        EXPECT_EQ(::access(pid_file, R_OK), 0); //! 文件应该存在
    }

    EXPECT_EQ(::access(pid_file, R_OK), -1);    //! 文件应该打不开，因为已经删除了
    EXPECT_EQ(errno, ENOENT);
}
