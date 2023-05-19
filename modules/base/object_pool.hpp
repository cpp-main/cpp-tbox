#ifndef TBOX_OBJECT_POOL_H
#define TBOX_OBJECT_POOL_H

#include <limits>

namespace tbox {

template <typename T>
class ObjectPool {
  public:
    explicit ObjectPool() { }
    explicit ObjectPool(size_t num) : keep_number_(num) { }

    ~ObjectPool() {
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

    template <typename ... Args>
    T* alloc(Args && ... args) {
        Block *block = free_header_;
        if (block == nullptr) {
            block = reinterpret_cast<Block*>(malloc(sizeof(Block)));
        } else {
            free_header_ = block->next;
            --free_number_;
        }

        T *p = reinterpret_cast<T*>(block);
        new (p) T(std::forward<Args>(args)...);
        return p;
    }

    void free(T* p) {
        p->~T();

        Block *block = reinterpret_cast<Block*>(p);
        if (free_number_ < keep_number_) {
            block->next = free_header_;
            free_header_ = block;
            ++free_number_;
        } else {
            ::free(block);
        }
    }

  private:
    size_t keep_number_ = std::numeric_limits<size_t>::max();
    size_t free_number_ = 0;
    Block *free_header_ = nullptr;
};

}

#endif //TBOX_OBJECT_POOL_H
