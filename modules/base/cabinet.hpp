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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_BASE_CABINET_HPP_20180415
#define TBOX_BASE_CABINET_HPP_20180415

/**
 * 实现一个对象储物柜 Cabinet
 * 其特征在于：
 * 1) 存储一个对象指针后，将获得一个 Token，即称:"凭据"；
 * 2) 该 Token 无法单位访问到对象，需要通过 Cabinet.at(token) 才能获取到对象；
 * 3) 如果 Token 对应的对象被删除，那么该 Token 就会失效。失效了的 Token 取不出对象；
 * 3) 使用 Token 获取对象、存入与取出对象的时间复杂度均为 O(1)；
 *
 * 使用场景：
 * - TcpServer 服务器，要管理其 TcpConnection 的生命期
 */

#include <cstdint>
#include <vector>
#include <limits>

#include "cabinet_token.h"

namespace tbox {
namespace cabinet {

//! 柜子
template <typename T> class Cabinet {
  public:
    void reserve(size_t size);  //!< 预留指定大小的储物空间

    Token alloc(T *obj = nullptr);  //!< 分配空间，并存入对象
    bool update(const Token &token, T *obj);    //! 更换对象
    T* free(const Token &token);    //!< 释放空间

    void clear();               //!< 清空所有对象

    //! 获取对象的指针
    T* at(const Token &token) const;
    T* operator [] (const Token &token) const { return at(token); }

    size_t size() const;
    bool empty() const { return size() == 0; }

    template <typename Func>
    void foreach(Func func);    //! 遍历过程中允许执行 remove() 操作

  protected:
    Id allocId();
    Pos allocPos();

  private:
    //! 单元格
    struct Cell {
        Id id = 0;  //!< 为 0 时，表示该格子为空闲状态，则 next_free 有效
                    //!< 不为 0 时，表示该格子存有对象，则 obj_ptr 有效
        union {
            T *obj_ptr = nullptr;   //!< 对象指针的地址
            Pos next_free;          //!< 下一个空闲格子的位置
        };
    };

    Id last_id_ = 0;    //!< 上一个被分配的id号
    std::vector<Cell> cells_;
    Pos first_free_ = std::numeric_limits<Pos>::max(); //!< 第一个空闲格式位置
    size_t count_ = 0;
};

///////////////////////////////////////////////////////////////////////

template <typename T>
void Cabinet<T>::reserve(size_t size)
{
    cells_.reserve(size);
}

template <typename T>
Token Cabinet<T>::alloc(T *obj)
{
    Token new_token(allocId(), allocPos());

    Cell &cell = cells_.at(new_token.pos());
    cell.id = new_token.id();
    cell.obj_ptr = obj;

    ++count_;
    return new_token;
}

template <typename T>
bool Cabinet<T>::update(const Token &token, T *obj)
{
    if (token.isNull() || token.pos() >= cells_.size())
        return false;

    Cell &cell = cells_.at(token.pos());
    if (cell.id == token.id()) {
        cell.obj_ptr = obj;
        return true;
    }
    return false;;
}

template <typename T>
T* Cabinet<T>::free(const Token &token)
{
    if (token.isNull() || token.pos() >= cells_.size())
        return nullptr;

    Cell &cell = cells_.at(token.pos());
    if (cell.id == token.id()) {
        T *ptr = cell.obj_ptr;
        cell.id = 0;
        cell.next_free = first_free_;
        first_free_ = token.pos();
        --count_;
        return ptr;
    }
    return nullptr;
}

template <typename T>
void Cabinet<T>::clear()
{
    last_id_ = 0;
    cells_.clear();
    first_free_ = std::numeric_limits<Pos>::max();
    count_ = 0;
}

template <typename T>
T* Cabinet<T>::at(const Token &token) const
{
    if (token.isNull() || token.pos() >= cells_.size())
        return nullptr;

    const Cell &cell = cells_.at(token.pos());
    if (cell.id == token.id())
        return cell.obj_ptr;
    return nullptr;
}

template <typename T>
size_t Cabinet<T>::size() const
{
    return count_;
}

template <typename T>
template <typename Func>
void Cabinet<T>::foreach(Func func)
{
    for (auto &cell : cells_) {
        if (cell.id != 0)
            func(cell.obj_ptr);
    }
}

template <typename T>
Id Cabinet<T>::allocId()
{
    //! 避免分配 0 作为 id
    if (last_id_ == std::numeric_limits<Id>::max())
        last_id_ = 0;

    return ++last_id_;
}

template <typename T>
Pos Cabinet<T>::allocPos()
{
    if (first_free_ != std::numeric_limits<Pos>::max()) {
        //! 如果有空间格子，则直接使用空间格式
        Pos new_pos = first_free_;
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

#endif //TBOX_BASE_CABINET_HPP_20180415
