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
/**
 * 本示例对 ObjectPool 与 new,delete 的时间耗时进行了统计，然后打印出两者平均耗时
 */
#include <tbox/base/object_pool.hpp>
#include <chrono>
#include <iostream>

struct MyStruct {
  public:
    MyStruct(int i) : i_(i) { }

    int i_;
    char array[100];
};

struct MyBlock {
    char array[1500];
};

int main() {
    tbox::ObjectPool<int> op1;
    tbox::ObjectPool<MyStruct> op2;
    tbox::ObjectPool<MyBlock> op3;

    std::chrono::nanoseconds acc_new = std::chrono::nanoseconds::zero();
    std::chrono::nanoseconds acc_op = std::chrono::nanoseconds::zero();

    std::chrono::steady_clock::time_point tp;
    constexpr int count = 100000;
    for (int i = 0; i < count; ++i) {
        tp = std::chrono::steady_clock::now();
        auto p01 = new MyStruct(i);
        auto p02 = new int(i);
        auto p03 = new MyBlock;
        acc_new += std::chrono::steady_clock::now() - tp;

        tp = std::chrono::steady_clock::now();
        auto p11 = op1.alloc(i);
        auto p12 = op2.alloc(i);
        auto p13 = op3.alloc();
        acc_op += std::chrono::steady_clock::now() - tp;

        tp = std::chrono::steady_clock::now();
        delete p01;
        delete p02;
        delete p03;
        acc_new += std::chrono::steady_clock::now() - tp;

        tp = std::chrono::steady_clock::now();
        op1.free(p11);
        op2.free(p12);
        op3.free(p13);
        acc_op += std::chrono::steady_clock::now() - tp;
    }

    std::cout << "new_acc: " << acc_new.count() / count / 3 << " ns" << std::endl;
    std::cout << "op_acc : " << acc_op.count() / count / 3 << " ns" << std::endl;

    return 0;
}
