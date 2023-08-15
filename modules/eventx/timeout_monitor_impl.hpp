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
#ifndef TBOX_UTIL_TIMEOUT_MONITOR_HPP_IMPL_20230218
#define TBOX_UTIL_TIMEOUT_MONITOR_HPP_IMPL_20230218

#include "timeout_monitor.hpp"

#include <tbox/base/assert.h>

namespace tbox {
namespace eventx {

template <typename T>
TimeoutMonitor<T>::TimeoutMonitor(event::Loop *wp_loop) :
    sp_timer_(wp_loop->newTimerEvent("TimeoutMonitor::sp_timer_"))
{ }

template <typename T>
TimeoutMonitor<T>::~TimeoutMonitor()
{
    TBOX_ASSERT(cb_level_ == 0);
    cleanup();
    delete sp_timer_;
}

template <typename T>
bool TimeoutMonitor<T>::initialize(const Duration &check_interval, int check_times)
{
    if (check_times < 1) {
        LogWarn("check_times should >= 1");
        return false;
    }

    sp_timer_->initialize(check_interval, event::Event::Mode::kPersist);
    sp_timer_->setCallback(std::bind(&TimeoutMonitor::onTimerTick, this));

    //! 创建计时环
    curr_item_ = new PollItem;
    auto last_item = curr_item_;
    for (int i = 1; i < check_times; ++i) {
        auto new_item = new PollItem;
        last_item->next = new_item;
        last_item = new_item;
    }
    last_item->next = curr_item_;

    return true;
}

template <typename T>
void TimeoutMonitor<T>::add(const T &value)
{
    curr_item_->items.push_back(value);
    if (value_number_ == 0)
        sp_timer_->enable();
    ++value_number_;
}

template <typename T>
void TimeoutMonitor<T>::cleanup()
{
    if (curr_item_ == nullptr)
        return;

    if (value_number_ > 0)
        sp_timer_->disable();
    value_number_ = 0;

    PollItem *item = curr_item_->next;
    curr_item_->next = nullptr;
    curr_item_ = nullptr;

    while (item != nullptr) {
        auto next = item->next;
        delete item;
        item = next;
    }

    cb_ = nullptr;
}

template <typename T>
void TimeoutMonitor<T>::onTimerTick()
{
    curr_item_ = curr_item_->next;

    std::vector<T> tobe_handle;
    std::swap(tobe_handle, curr_item_->items);

    value_number_ -= tobe_handle.size();
    if (value_number_ == 0)
        sp_timer_->disable();

    if (cb_) {
        ++cb_level_;
        for (auto value : tobe_handle)
            cb_(value);
        --cb_level_;
    }
}

}
}

#endif //TBOX_UTIL_TIMEOUT_MONITOR_HPP_IMPL_20230218
