#include <gtest/gtest.h>
#include "object_container.hpp"
#include <vector>

using namespace tbox;

TEST(ObjectContainer, insert_1_and_remove)
{
    ObjectContainer<int> oc;
    auto t = oc.insert(new int(100));

    int *i1 = oc.at(t);
    EXPECT_EQ(oc.size(), 1u);
    EXPECT_NE(i1, nullptr);
    EXPECT_EQ(*i1, 100);

    oc.erase(t);
    EXPECT_EQ(oc.size(), 0u);
    delete i1;

    int *i2 = oc.at(t);
    EXPECT_EQ(i2, nullptr);
}

TEST(ObjectContainer, insert_100_and_remove)
{
    using OC = ObjectContainer<int>;
    OC oc;

    std::vector<OC::Token> tokens;
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

        oc.erase(t);
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

        oc.erase(t);
        delete p;
    }
    EXPECT_EQ(oc.size(), 0u);
}

