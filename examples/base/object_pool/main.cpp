#include <tbox/base/object_pool.hpp>
#include <chrono>
#include <iostream>

struct MyStruct {
  public:
    MyStruct(int i) : i_(i) { }
  private:
    int i_;
    char ca[50];
};


int main() {
    tbox::ObjectPool<int> op1;
    tbox::ObjectPool<MyStruct> op2;

    std::chrono::nanoseconds acc_new = std::chrono::nanoseconds::zero();
    std::chrono::nanoseconds acc_op = std::chrono::nanoseconds::zero();

    std::chrono::steady_clock::time_point tp;
    constexpr int count = 1000000;
    for (int i = 0; i < count; ++i) {
        tp = std::chrono::steady_clock::now();
        auto p1 = new MyStruct(i);
        auto p2 = new int(i);
        acc_new += std::chrono::steady_clock::now() - tp;

        tp = std::chrono::steady_clock::now();
        auto p3 = op1.alloc(i);
        auto p4 = op2.alloc(i);
        acc_op += std::chrono::steady_clock::now() - tp;

        tp = std::chrono::steady_clock::now();
        delete p1;
        delete p2;
        acc_new += std::chrono::steady_clock::now() - tp;

        tp = std::chrono::steady_clock::now();
        op1.free(p3);
        op2.free(p4);
        acc_op += std::chrono::steady_clock::now() - tp;
    }

    std::cout << "new_acc: " << acc_new.count() / count << " ns" << std::endl;
    std::cout << "op_acc : " << acc_op.count() / count << " ns" << std::endl;

    return 0;
}
