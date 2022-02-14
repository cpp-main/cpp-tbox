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

    inline bool operator == (const Token &other) const { return equal(other); }
    inline bool operator < (const Token &other) const { return less(other); }

  private:
    Id id_ = 0;
    Pos pos_ = 0;
};

}
}

#endif //TBOX_BASE_CABINET_TOKEN_H_20220214
