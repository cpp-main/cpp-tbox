#ifndef TBOX_OBJECT_POOL_H
#define TBOX_OBJECT_POOL_H

#include <cstdlib>
#include <utility>
#include <limits>

namespace tbox {

/**
 * ObjectPool，对象池
 *
 * 用于减少特定场景下频繁 new 与 delete 对象过程的性能。
 * 实现原理是：
 * - 在 ObjectPool 中有一个空闲块链表；
 * - 在 free() 时，它将对象的内存块直接存入到空闲块链表缓存起来；
 * - 在 alloc() 时，它直接取用空闲块链表中的块来使用，如果没有才会执行 malloc() 去分配；
 * 从而使用避免了反复分配与释放内存的过程。
 *
 * 使用示例：
 * -----------------------------------------------------------------
 * class MyStruct {
 *   public:
 *     MyString(int i, const std::string &s) : i_(i), s_(s) { }
 *     void print() { cout << "i:" << i_ << ", s:" << s_ << endl; }
 *   private
 *     int i_;
 *     std::string s_;
 * };
 *
 * ObjectPool<MyStruct> op;
 * ...
 * auto p1 = op.alloc(1, "hello");
 * // 等价于：auto p1 = new MyStruct(1, "hello");
 * p1->print();
 * op.free(p1);
 * // 等价于：delete p1;
 * -----------------------------------------------------------------
 *
 * 注意：
 * - 凡是使用 ObjectPool 分配的对象，一定要使用 ObjectPool 进行释放
 */
template <typename T>
class ObjectPool {
  public:
    explicit ObjectPool() { }
    explicit ObjectPool(size_t num) : keep_number_(num) { }

    ~ObjectPool() {
        //! 释放掉所有的空闲块
        while (free_header_ != nullptr) {
            auto next = free_header_->next;
            ::free(free_header_);
            free_header_ = next;
        }
    }

  public:
    union Block {
        Block *next;
        char reserve[sizeof(T)];
    };

    //! new 一个对象
    template <typename ... Args>
    T* alloc(Args && ... args) {
        Block *block = free_header_;
        if (block == nullptr) {
            //! 如果空闲块链表为空，就调 malloc() 进行分配
            block = reinterpret_cast<Block*>(malloc(sizeof(Block)));
        } else {
            //! 直接从空闲块链表取出一块
            free_header_ = block->next;
            --free_number_;
        }

        T *p = reinterpret_cast<T*>(block);
        //! 执行对象的构造函数
        new (p) T(std::forward<Args>(args)...);
        return p;
    }

    //! delete 指定对象
    void free(T* p) {
        p->~T();    //! 执行对象的析构函数

        Block *block = reinterpret_cast<Block*>(p);
        if (free_number_ < keep_number_) {
            //! 如果空闲块链表还没有足够多的空闲块，就插入链表
            block->next = free_header_;
            free_header_ = block;
            ++free_number_;
        } else {
            //! 否则就直接释放掉
            ::free(block);
        }
    }

  private:
    size_t keep_number_ = std::numeric_limits<size_t>::max();
    size_t free_number_ = 0;        //!< 空闲块数量
    Block *free_header_ = nullptr;  //!< 空闲块链表
};

}

#endif //TBOX_OBJECT_POOL_H
