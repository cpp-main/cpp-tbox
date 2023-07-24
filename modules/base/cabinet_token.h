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
#ifndef TBOX_BASE_CABINET_TOKEN_H_20220214
#define TBOX_BASE_CABINET_TOKEN_H_20220214

#include <cstdlib>

namespace tbox {
namespace cabinet {

using Id = size_t;
using Pos = size_t;

//! 凭据
class Token {
  public:
    Token() { }
    Token(Id id, Pos pos) : id_(id), pos_(pos) { }

    inline Id id() const { return id_; }
    inline Pos pos() const { return pos_; }

    inline void reset() { id_ = 0; pos_ = 0; }
    inline bool isNull() const { return id_ == 0; }

    inline bool equal(const Token &other) const { return id_ == other.id_ && pos_ == other.pos_; }
    inline bool less(const Token &other)  const { return id_ != other.id_ ? id_ < other.id_ : pos_ < other.pos_; }
    inline size_t hash() const { return (id_ << 8) | (pos_ & 0xff); }

    inline bool operator == (const tbox::cabinet::Token &rhs) const { return equal(rhs); }
    inline bool operator != (const tbox::cabinet::Token &rhs) const { return !equal(rhs); }
    inline bool operator <  (const tbox::cabinet::Token &rhs) const { return less(rhs); }
    inline bool operator <= (const tbox::cabinet::Token &rhs) const { return less(rhs) || equal(rhs); }
    inline bool operator >  (const tbox::cabinet::Token &rhs) const { return !less(rhs) && !equal(rhs); }
    inline bool operator >= (const tbox::cabinet::Token &rhs) const { return !less(rhs); }

    inline operator bool () const { return id_ != 0; }

  private:
    Id id_ = 0;
    Pos pos_ = 0;
};

}
}

//! 为了支持 unordered_set 与 unordered_map 的 key
namespace std {
template <class T> struct hash;
template <> struct hash <tbox::cabinet::Token> {
    size_t operator () (const tbox::cabinet::Token &t) const {
        return t.hash();
    }
};
}

#endif //TBOX_BASE_CABINET_TOKEN_H_20220214
