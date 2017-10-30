#include "fd.h"

#include <gtest/gtest.h>
using namespace tbox::network;

TEST(network_fd, swap)
{
    Fd fd(12);
    ASSERT_EQ(fd.get(), 12);
    Fd other(13);
    fd.swap(other);
    EXPECT_EQ(fd.get(), 13);
    EXPECT_EQ(other.get(), 12);
}

TEST(network_fd, move_1)
{
    Fd fd1(12);
    Fd fd2(13);
    fd1 = std::move(fd2);
    EXPECT_EQ(fd1.get(), 13);
    EXPECT_EQ(fd2.get(), -1);
}

TEST(network_fd, move_2)
{
    Fd fd1(12);
    Fd fd2(std::move(fd1));
    EXPECT_EQ(fd2.get(), 12);
    EXPECT_EQ(fd1.get(), -1);
}

TEST(network_fd, reset)
{
    Fd fd(12);
    fd.reset();
    EXPECT_EQ(fd.get(), -1);
}

TEST(network_fd, cast)
{
    Fd fd(12);
    EXPECT_EQ(fd, 12);
}
