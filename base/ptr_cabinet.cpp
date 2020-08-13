#include "ptr_cabinet.h"

namespace tbox {
namespace ptr_cabinet {

void Cabinet::reserve(size_t size)
{
    cells_.reserve(size);
}

Token Cabinet::insert(void *ptr)
{
    Token new_token;
    new_token.id = allocId();
    new_token.pos = allocPos();

    Cell &cell = cells_.at(new_token.pos);
    cell.id = new_token.id;
    cell.ptr = ptr;

    ++count_;
    return new_token;
}

void* Cabinet::remove(const Token &token)
{
    Cell &cell = cells_.at(token.pos);
    if (cell.id == token.id) {
        void *ptr = cell.ptr;
        cell.id = 0;
        cell.next_free = first_free_;
        first_free_ = token.pos;
        --count_;
        return ptr;
    }
    return nullptr;
}

void Cabinet::clear()
{
    last_id_ = 0;
    cells_.clear();
    first_free_ = std::numeric_limits<size_t>::max();
    count_ = 0;
}

void* Cabinet::at(const Token &token) const
{
    const Cell &cell = cells_.at(token.pos);
    if (cell.id == token.id)
        return cell.ptr;
    return nullptr;
}

size_t Cabinet::size() const
{
    return count_;
}

void Cabinet::foreach(const ForeachFunc &func)
{
    for (auto &cell : cells_) {
        if (cell.id != 0)
            func(cell.ptr);
    }
}

Id Cabinet::allocId()
{
    //! 避免分配 0 作为 id
    if (last_id_ == std::numeric_limits<Id>::max())
        last_id_ = 0;

    return ++last_id_;
}

size_t Cabinet::allocPos()
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
}
