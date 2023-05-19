#include <gtest/gtest.h>
#include "object_pool.hpp"

namespace tbox {
namespace {

TEST(ObjectPool, Char) {
    ObjectPool<char> pool;
    for (int i = 0; i < 10; ++i) {
        auto p = pool.alloc('a' + i);
        EXPECT_EQ(*p, 'a' + i);
        pool.free(p);
    }
}

TEST(ObjectPool, AllocMany) {
    ObjectPool<int> pool(50);
    std::vector<int*> tmp;

    for (int i = 0; i < 100; ++i) {
        auto p = pool.alloc(i);
        tmp.push_back(p);
    }

    for (int *p : tmp)
        pool.free(p);
    tmp.clear();

    for (int i = 0; i < 50; ++i) {
        auto p = pool.alloc(i);
        tmp.push_back(p);
    }

    for (int *p : tmp)
        pool.free(p);
    tmp.clear();
}

TEST(ObjectPool, Int) {
    ObjectPool<int> pool;
    for (int i = 0; i < 10; ++i) {
        auto p = pool.alloc(123);
        EXPECT_EQ(*p, 123);
        pool.free(p);
    }
}

TEST(ObjectPool, MyStruct) {
    struct MyStruct {
        MyStruct(int i) : i_(i) { }
        char rev[100];
        int i_;
    };

    ObjectPool<MyStruct> pool;
    for (int i = 0; i < 10; ++i) {
        auto p = pool.alloc(123);
        EXPECT_EQ(p->i_, 123);
        pool.free(p);
    }
}

}
}
