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
    void bind() { token_ = sch_.getToken(); }
    //! 绑定指定协程
    void bind(const RoutineToken &token) { token_ = token; }

    Condition& operator = (bool v) {
        if (v && !v_)
            sch_.resume(token_);
        v_ = v;
        return *this;
    }

    operator bool () const { return v_; }

  private:
    Scheduler &sch_;
    RoutineToken token_;

    bool v_ = false;
};

}
}

#endif //TBOX_COROUTINE_CONDITION_H_20180526
