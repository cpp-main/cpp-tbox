#include <gtest/gtest.h>
#include "cabinet.hpp"
#include <vector>
#include <unordered_set>
#include <set>
#include <unordered_set>

using namespace tbox;
using namespace tbox::cabinet;

TEST(Cabinet, insert_1_and_remove)
{
    Cabinet<int> oc;
    auto t = oc.insert(new int(100));

    int *i1 = oc.at(t);
    EXPECT_EQ(oc.size(), 1u);
    EXPECT_NE(i1, nullptr);
    EXPECT_EQ(*i1, 100);

    oc.remove(t);
    EXPECT_EQ(oc.size(), 0u);
    delete i1;

    int *i2 = oc.at(t);
    EXPECT_EQ(i2, nullptr);
}

TEST(Cabinet, insert_100_and_remove)
{
    using OC = Cabinet<int>;
    OC oc;

    std::vector<Token> tokens;
    //! 插入0~74的值
    for (int i = 0; i < 75; ++i) {
        auto t = oc.insert(new int(i));
        tokens.push_back(t);
    }
    EXPECT_EQ(oc.size(), 75u);

    //! 读取前50个，应该都能读到。然后都删除元素
    for (int i = 0; i < 50; ++i) {
        auto t = tokens[i];
        auto *p = oc.at(t);
        EXPECT_NE(p, nullptr);
        EXPECT_EQ(*p, i);

        oc.remove(t);
        delete p;
    }
    EXPECT_EQ(oc.size(), 25u);

    //! 再次读取前50个，token应该都失效了
    for (int i = 0; i < 50; ++i) {
        auto t = tokens[i];
        auto *p = oc.at(t);
        EXPECT_EQ(p, nullptr);
    }

    //! 插入75~99的值
    for (int i = 75; i < 100; ++i) {
        auto t = oc.insert(new int(i));
        tokens.push_back(t);
    }
    EXPECT_EQ(oc.size(), 50u);

    //! 读取后50个，应该都能读到。然后都删除元素
    for (int i = 50; i < 100; ++i) {
        auto t = tokens[i];
        auto *p = oc.at(t);
        EXPECT_NE(p, nullptr);
        EXPECT_EQ(*p, i);

        oc.remove(t);
        delete p;
    }
    EXPECT_EQ(oc.size(), 0u);
}

TEST(Cabinet, alloc_update)
{
    cabinet::Cabinet<int> c;
    auto t = c.alloc();
    EXPECT_EQ(c.at(t), nullptr);

    int *p = new int (10);
    c.update(t, p);
    EXPECT_EQ(c.at(t), p);

    delete p;
}

//! 测试Token可不可以用为std::set的key
TEST(Cabinet, token_as_set_key)
{
    std::set<Token> token_set;
    token_set.insert(Token(1, 2));
    token_set.insert(Token(2, 0));
    EXPECT_EQ(token_set.size(), 2u);
}

//! 测试Token可不可以用为std::unordered_set的key
TEST(Cabinet, token_as_unordered_set_key)
{
    std::unordered_set<Token> token_set;
    token_set.insert(Token(1, 2));
    token_set.insert(Token(2, 0));
    EXPECT_EQ(token_set.size(), 2u);
}
