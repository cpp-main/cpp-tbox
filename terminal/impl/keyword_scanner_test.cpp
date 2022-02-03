#include <gtest/gtest.h>

#include "keyword_scanner.h"

using namespace tbox::terminal;

TEST(KeywordScanner, _)
{
    KeywordScanner ks;

    ks.addKeyword({1, 2}, 1);
    ks.addKeyword({1, 3}, 2);

    EXPECT_EQ(ks.feed(1), 0);
    EXPECT_EQ(ks.feed(2), 1);

    EXPECT_EQ(ks.feed(1), 0);
    EXPECT_EQ(ks.feed(3), 2);
}
