#ifndef TBOX_BASE_OBJECT_CONTAINER_H_20180415
#define TBOX_BASE_OBJECT_CONTAINER_H_20180415

/**
 * 实现一个对象容器 ObjectContainer
 * 其特征在于：
 * 1) 往其插入一个对象指针后，它返回一个 Token，即称:"票据"；
 * 2) 该 Token 无法单位访问到对象，需要通过 ObjectContainer.at(token) 才能获取到对象，也有可能该对象已失效；
 * 3) 使用 Token 获取对象、插入与删除对象的时间复杂度均为 O(1)；
 *
 * 使用场景：
 * - TcpServer 服务器，要管理其 TcpConnection 的生命期
 */

#include <cstdlib>
#include <cstdint>
#include <vector>
#include <limits>

namespace tbox {

template <typename T> class ObjectContainer {
  public:
    using Id = uint32_t;
    struct Token {
        Id id;
        size_t pos;
    };

    void reserve(size_t size);

    Token insert(T *obj);
    bool erase(const Token &token);
    void clear();

    T* at(const Token &token) const;
    T* operator [] (const Token &token) const { return at(token); }

    size_t size() const;
    bool empty() const { return size() == 0; }

    template <typename Func>
    void foreach(Func func);

  protected:
    Id allocId();
    size_t allocPos();

  private:
    struct Item {
        Id id = 0;
        union {
            T *obj_ptr = nullptr;
            size_t next_free;
        };
    };

    Id last_id_ = 0;
    std::vector<Item> objs_;
    size_t first_free_ = std::numeric_limits<size_t>::max();
    size_t count_ = 0;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
void ObjectContainer<T>::reserve(size_t size)
{
    objs_.reserve(size);
}

template <typename T>
typename ObjectContainer<T>::Token ObjectContainer<T>::insert(T *obj)
{
    Token new_token;
    new_token.id = allocId();
    new_token.pos = allocPos();

    Item &item = objs_.at(new_token.pos);
    item.id = new_token.id;
    item.obj_ptr = obj;

    ++count_;
    return new_token;
}

template <typename T>
bool ObjectContainer<T>::erase(const ObjectContainer<T>::Token &token)
{
    Item &item = objs_.at(token.pos);
    if (item.id == token.id) {
        item.id = 0;
        item.next_free = first_free_;
        first_free_ = token.pos;

        --count_;
        return true;
    }
    return false;
}

template <typename T>
void ObjectContainer<T>::clear()
{
    last_id_ = 0;
    objs_.clear();
    first_free_ = std::numeric_limits<size_t>::max();
    count_ = 0;
}

template <typename T>
T* ObjectContainer<T>::at(const ObjectContainer<T>::Token &token) const
{
    const Item &item = objs_.at(token.pos);
    if (item.id == token.id)
        return item.obj_ptr;
    return nullptr;
}

template <typename T>
size_t ObjectContainer<T>::size() const
{
    return count_;
}

template <typename T>
template <typename Func>
void ObjectContainer<T>::foreach(Func func)
{
    for (auto &item : objs_) {
        if (item.id != 0)
            func(item.obj_ptr);
    }
}

template <typename T>
typename ObjectContainer<T>::Id ObjectContainer<T>::allocId()
{
    //! 0 is not allow
    if (last_id_ == std::numeric_limits<Id>::max())
        last_id_ = 0;

    return ++last_id_;
}

template <typename T>
size_t ObjectContainer<T>::allocPos()
{
    if (first_free_ != std::numeric_limits<size_t>::max()) {
        size_t new_pos = first_free_;
        Item &item = objs_.at(new_pos);
        first_free_ = item.next_free;
        return new_pos;
    } else {
        objs_.push_back(Item());
        return objs_.size() - 1;
    }
}

}

#endif //TBOX_BASE_OBJECT_CONTAINER_H_20180415
