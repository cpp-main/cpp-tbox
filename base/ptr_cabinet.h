#ifndef TBOX_BASE_CABINET_H_20200813
#define TBOX_BASE_CABINET_H_20200813

#include <cstdlib>
#include <cstdint>

#include <vector>
#include <limits>
#include <functional>

namespace tbox {
namespace ptr_cabinet {

using Id = size_t;

class Cabinet;

class Token {
    friend Cabinet;

    Id id = 0;
    size_t pos = 0;

    public:
    inline void reset() { id = 0; pos = 0; }
    inline bool isNull() const { return id == 0; }

    inline bool equal(const Token &other) const { return id == other.id && pos == other.pos; }
    inline bool less(const Token &other)  const { return id != other.id ? id < other.id : pos < other.pos; }
    inline size_t hash() const { return (id << 8) | (pos & 0xff); }

    Id getId() const { return id; }
};

class Cabinet {
  public:
    void reserve(size_t size);  //!< 预留指定大小的储物空间

    Token insert(void *ptr);            //!< 存入指针
    void* remove(const Token &token);   //!< 删除指针
    void clear();                       //!< 清空所有指针

    //! 获取指针的指针
    void* at(const Token &token) const;
    void* operator [] (const Token &token) const { return at(token); }

    size_t size() const;
    bool empty() const { return size() == 0; }

    using ForeachFunc = std::function<void(void*)>;
    void foreach(const ForeachFunc &func); //! 遍历过程中允许执行 remove() 操作

  protected:
    Id allocId();
    size_t allocPos();

  private:
    struct Cell {
        Id id = 0;  //!< 为 0 时，表示该格子为空闲状态，则 next_free 有效
                    //!< 不为 0 时，表示该格子存有指针，则 ptr 有效
        union {
            void  *ptr = nullptr;   //!< 指针指针的地址
            size_t next_free;       //!< 下一个空闲格子的位置
        };
    };

    Id last_id_ = 0;    //!< 上一个被分配的id号
    std::vector<Cell> cells_;
    size_t first_free_ = std::numeric_limits<size_t>::max(); //!< 第一个空闲格式位置
    size_t count_ = 0;
};

}
}

#endif //TBOX_BASE_CABINET_H_20200813
