#ifndef TBOX_COROUTINE_MUTEX_HPP_20180527
#define TBOX_COROUTINE_MUTEX_HPP_20180527

#include "scheduler.h"

namespace tbox {
namespace coroutine {

//! 信号量
class Mutex {
  public:
    Mutex(Scheduler &sch) : sch_(sch) { }

    //! 请求资源，注意：只能是协程调用
    bool lock() {
        if (!hold_token_.isNull()) {      //! 如果没有资源，则等待
            wait_tokens_.push(sch_.getToken());
            do {
                sch_.wait();
                if (sch_.isCanceled())
                    return false;
            } while (!hold_token_.isNull());
        }

        hold_token_ = sch_.getToken();
        return true;
    }

    //! 释放资源
    void unlock() {
        if (!sch_.getToken().equal(hold_token_))
            return;
        hold_token_.reset();

        if (!wait_tokens_.empty()) {
            auto t = wait_tokens_.front();
            wait_tokens_.pop();
            sch_.resume(t);
        }
    }

  private:
    Scheduler &sch_;

    RoutineToken hold_token_;
    std::queue<RoutineToken> wait_tokens_;
};

}
}

#endif //TBOX_COROUTINE_MUTEX_HPP_20180527
