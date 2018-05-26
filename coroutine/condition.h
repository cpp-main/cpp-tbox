#ifndef TBOX_COROUTINE_CONDITION_H_20180526
#define TBOX_COROUTINE_CONDITION_H_20180526

#include "scheduler.h"

namespace tbox {
namespace coroutine {

//! 条件类
class Condition {
  public:
    Condition(Scheduler &sch) : sch_(sch) { }

    //! 绑定当前协程
    void bind() { key_ = sch_.getKey(); }
    //! 绑定指定协程
    void bind(const RoutineKey &key) { key_ = key; }

    Condition& operator = (bool v) {
        if (v && !v_)
            sch_.resume(key_);
        v_ = v;
        return *this;
    }

    operator bool () const { return v_; }

  private:
    Scheduler &sch_;
    RoutineKey key_;

    bool v_ = false;
};

}
}

#endif //TBOX_COROUTINE_CONDITION_H_20180526
