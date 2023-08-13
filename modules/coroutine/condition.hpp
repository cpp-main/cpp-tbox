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
#ifndef TBOX_COROUTINE_CONDITION_HPP_20180605
#define TBOX_COROUTINE_CONDITION_HPP_20180605

#include <set>
#include "scheduler.h"

namespace tbox {
namespace coroutine {

/**
 * 条件，可以同时等待多个条件同时满足，也可以等待多个条件任一满足
 *
 * 使用示例：
 *   协程A要在协程B与协程C都完成了某个动作后才能执行，这时候使用All条件
 */
template <class T>
class Condition {
  public:
    enum class Logic {
        kAll,   //! 满足所有
        kAny    //! 满足任意
    };

    Condition(Scheduler &sch, Logic logic) : sch_(sch), logic_(logic) { }

    void add(T v) {
        conds_.insert(v);
    }

    bool wait() {
        //! 不支持多个协程同时等，也不允许在没的指定条件的情况下等
        if (!wait_token_.isNull() || conds_.empty())
            return false;

        wait_token_ = sch_.getToken();
        sch_.wait();
        conds_.clear();

        return !sch_.isCanceled();
    }

    void post(T v) {
        if (conds_.find(v) == conds_.end())
            return;

        if (logic_ == Logic::kAll) {
            conds_.erase(v);
            if (!conds_.empty())
                return;
        } else {
            conds_.clear();
        }

        if (!wait_token_.isNull())
            sch_.resume(wait_token_);
        wait_token_.reset();
    }

  private:
    Scheduler &sch_;
    Logic logic_;
    std::set<T> conds_;
    RoutineToken wait_token_;
};

}
}

#endif //TBOX_COROUTINE_CONDITION_HPP_20180605
