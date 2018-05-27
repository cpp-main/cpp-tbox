#ifndef TBOX_COROUTINE_CHANNEL_HPP_20180527
#define TBOX_COROUTINE_CHANNEL_HPP_20180527

#include "scheduler.h"
#include <queue>

namespace tbox {
namespace coroutine {

//! 参考 Golang 的 chan，实现一个通道
template <class T>
class Channel {
  public:
    Channel (Scheduler &sch) : sch_(sch) { }

    bool operator >> (T &out) {
        if (queue_.empty()) {   //! 如果队列里没有，则等待
            token_.push(sch_.getToken());
            sch_.wait();

            if (queue_.empty()) //! 检查一下，有可能协程被cancel
                return false;
        }

        out = queue_.front();
        queue_.pop();
        return true;
    }

    Channel& operator << (const T &value) {
        if (queue_.empty() && !token_.empty()) {
            auto t = token_.front();
            token_.pop();
            sch_.resume(t);
        }
        queue_.push(value);
        return *this;
    }

    inline bool empty() const { return queue_.empty(); }
    inline bool size() const { return queue_.size(); }

  private:
    Scheduler &sch_;

    std::queue<T> queue_;
    std::queue<RoutineToken> token_;
};

}
}

#endif //TBOX_COROUTINE_CHANNEL_HPP_20180527
