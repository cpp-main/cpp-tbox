#include <gtest/gtest.h>
#include "cabinet_token.h"
#include <vector>
#include <unordered_set>
#include <set>

namespace tbox {
namespace cabinet {
namespace {

TEST(CabinetToken, NullToken)
{
    Token token;
    EXPECT_TRUE(token.isNull());
    EXPECT_FALSE(token);
}

TEST(CabinetToken, NotNullToken)
{
    Token token(1, 2);
    EXPECT_FALSE(token.isNull());
    EXPECT_TRUE(token);
}

//! 测试Token可不可以用为std::set的key
TEST(CabinetToken, token_as_set_key)
{
    std::set<Token> token_set;
    token_set.insert(Token(1, 2));
    token_set.insert(Token(2, 0));
    EXPECT_EQ(token_set.size(), 2u);
}

//! 测试Token可不可以用为std::unordered_set的key
TEST(CabinetToken, token_as_unordered_set_key)
{
    std::unordered_set<Token> token_set;
    token_set.insert(Token(1, 2));
    token_set.insert(Token(2, 0));
    EXPECT_EQ(token_set.size(), 2u);
}

}
}
}
