#ifndef TBOX_BASE_OBJECT_LOCKER_H_20180415
#define TBOX_BASE_OBJECT_LOCKER_H_20180415

/**
 * 实现一个对象储物柜 ObjectLocker
 * 其特征在于：
 * 1) 存储一个对象指针后，将获得一个 Key，即称:"钥匙"；
 * 2) 该 Key 无法单位访问到对象，需要通过 ObjectLocker.at(key) 才能获取到对象；
 * 3) 如果 Key 对应的对象被删除，那么该 Key 就会失效。失效了的 Key 取不出对象；
 * 3) 使用 Key 获取对象、存入与取走对象的时间复杂度均为 O(1)；
 *
 * 使用场景：
 * - TcpServer 服务器，要管理其 TcpConnection 的生命期
 */

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <limits>

namespace tbox {

template <typename T> class ObjectLocker {
  public:
    using Id = size_t;

    class Key {
        friend ObjectLocker;

        Id id;
        size_t pos;

      public:
        inline bool equal(const Key &other) const { return id == other.id && pos == other.pos; }
        inline bool less(const Key &other)  const { return id != other.id ? id < other.id : pos < other.pos; }
        inline size_t hash() const { return (id << 8) | (pos & 0xff); }

        Id getId() const { return id; }
    };

    void reserve(size_t size);  //!< 预留指定大小的储物空间

    Key insert(T *obj);         //!< 存入对象
    T*  remove(const Key &key); //!< 删除对象
    void clear();               //!< 清空所有对象

    //! 获取对象的指针
    T* at(const Key &key) const;
    T* operator [] (const Key &key) const { return at(key); }

    size_t size() const;
    bool empty() const { return size() == 0; }

    template <typename Func>
    void foreach(Func func);    //! 遍历过程中允许执行 remove() 操作

  protected:
    Id allocId();
    size_t allocPos();

  private:
    struct Cell {
        Id id = 0;  //!< 为 0 时，表示该格子为空闲状态，则 next_free 有效
                    //!< 不为 0 时，表示该格子存有对象，则 obj_ptr 有效
        union {
            T *obj_ptr = nullptr;   //!< 对象指针的地址
            size_t next_free;       //!< 下一个空闲格子的位置
        };
    };

    Id last_id_ = 0;    //!< 上一个被分配的id号
    std::vector<Cell> cells_;
    size_t first_free_ = std::numeric_limits<size_t>::max(); //!< 第一个空闲格式位置
    size_t count_ = 0;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
void ObjectLocker<T>::reserve(size_t size)
{
    cells_.reserve(size);
}

template <typename T>
typename ObjectLocker<T>::Key ObjectLocker<T>::insert(T *obj)
{
    Key new_key;
    new_key.id = allocId();
    new_key.pos = allocPos();

    Cell &cell = cells_.at(new_key.pos);
    cell.id = new_key.id;
    cell.obj_ptr = obj;

    ++count_;
    return new_key;
}

template <typename T>
T* ObjectLocker<T>::remove(const ObjectLocker<T>::Key &key)
{
    Cell &cell = cells_.at(key.pos);
    if (cell.id == key.id) {
        T *ptr = cell.obj_ptr;
        cell.id = 0;
        cell.next_free = first_free_;
        first_free_ = key.pos;
        --count_;
        return ptr;
    }
    return nullptr;
}

template <typename T>
void ObjectLocker<T>::clear()
{
    last_id_ = 0;
    cells_.clear();
    first_free_ = std::numeric_limits<size_t>::max();
    count_ = 0;
}

template <typename T>
T* ObjectLocker<T>::at(const ObjectLocker<T>::Key &key) const
{
    const Cell &cell = cells_.at(key.pos);
    if (cell.id == key.id)
        return cell.obj_ptr;
    return nullptr;
}

template <typename T>
size_t ObjectLocker<T>::size() const
{
    return count_;
}

template <typename T>
template <typename Func>
void ObjectLocker<T>::foreach(Func func)
{
    for (auto &cell : cells_) {
        if (cell.id != 0)
            func(cell.obj_ptr);
    }
}

template <typename T>
typename ObjectLocker<T>::Id ObjectLocker<T>::allocId()
{
    //! 避免分配 0 作为 id
    if (last_id_ == std::numeric_limits<Id>::max())
        last_id_ = 0;

    return ++last_id_;
}

template <typename T>
size_t ObjectLocker<T>::allocPos()
{
    if (first_free_ != std::numeric_limits<size_t>::max()) {
        //! 如果有空间格子，则直接使用空间格式
        size_t new_pos = first_free_;
        Cell &cell = cells_.at(new_pos);
        first_free_ = cell.next_free;
        return new_pos;
    } else {
        //! 否则在尾部追加一个格子
        cells_.push_back(Cell());
        return cells_.size() - 1;
    }
}

}

#endif //TBOX_BASE_OBJECT_LOCKER_H_20180415
