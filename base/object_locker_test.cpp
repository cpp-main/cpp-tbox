#include <gtest/gtest.h>
#include "object_locker.hpp"
#include <vector>
#include <unordered_set>
#include <set>

using namespace tbox;

TEST(ObjectLocker, insert_1_and_remove)
{
    ObjectLocker<int> oc;
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

TEST(ObjectLocker, insert_100_and_remove)
{
    using OC = ObjectLocker<int>;
    OC oc;

    std::vector<OC::Key> tokens;
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

#if 0
TEST(ObjectLocker, key)
{
    using Key = ObjectLocker<int>::Key;
    std::set<Key> s1;
    std::unordered_set<Key> s2;
}
#endif
