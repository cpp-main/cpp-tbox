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
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <memory>
#include <algorithm>
#include <atomic>

#include "loop_wdog.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/defines.h>
#include <tbox/base/wrapped_recorder.h>

namespace tbox {
namespace eventx {

namespace {

void OnLoopBlock(const std::string &name);
void OnLoopRecover(const std::string &name);

//! Loop线程的状态
enum class LoopState {
    kTobeCheck, //! 未打卡
    kChecked,   //! 已打卡
    kTimeout    //! 打卡超时
};

//! WDog自身的状态
enum class WDogState {
    kIdle,      //!< 未启动
    kRunning,   //!< 运行中
    kStoping,   //!< 停止中
};

struct LoopInfo {
    using SharedPtr = std::shared_ptr<LoopInfo>;

    LoopInfo(event::Loop *l, const std::string &n) :
        loop(l), name(n),
        state(LoopState::kChecked)
    { }

    event::Loop*  loop;
    std::string   name;
    LoopState     state;
};

using LoopInfoVec = std::vector<LoopInfo::SharedPtr>;

LoopInfoVec     _loop_info_vec; //! 线程信息表
std::mutex      _mutex_lock;    //! 锁
std::thread*    _sp_thread = nullptr; //! 线程对象
std::atomic<WDogState> _wdog_state(WDogState::kIdle);

LoopWDog::LoopBlockCallback _loop_die_cb = OnLoopBlock;  //! 回调函数
LoopWDog::LoopBlockCallback _loop_recover_cb = OnLoopRecover;  //! 回调函数

void SendLoopFunc() {
    RECORD_SCOPE();
    std::lock_guard<std::mutex> lg(_mutex_lock);
    for (auto loop_info_sptr : _loop_info_vec) {
        if (loop_info_sptr->loop->isRunning()) {
            if (loop_info_sptr->state == LoopState::kChecked) {
                loop_info_sptr->state = LoopState::kTobeCheck;

                auto start_ts = std::chrono::steady_clock::now();

                loop_info_sptr->loop->runInLoop(
                    [loop_info_sptr] {
                        if (loop_info_sptr->state == LoopState::kTimeout)
                            _loop_recover_cb(loop_info_sptr->name);
                        loop_info_sptr->state = LoopState::kChecked;
                    },
                    "LoopWdog"
                );

                auto time_cost = std::chrono::steady_clock::now() - start_ts;
                if (time_cost > std::chrono::milliseconds(100)) {
                    LogWarn("loop \"%s\" runInLoop() cost > 100 ms, %lu us", loop_info_sptr->name.c_str(), time_cost.count() / 1000);
                }
            }
        }
    }
}

void CheckLoopTag() {
    std::lock_guard<std::mutex> lg(_mutex_lock);
    for (auto loop_info_sptr: _loop_info_vec) {
        if (loop_info_sptr->state == LoopState::kTobeCheck) {
            loop_info_sptr->state = LoopState::kTimeout;
            _loop_die_cb(loop_info_sptr->name);
            RECORD_EVENT();
        }
    }
}

//! 监控线程函数
void ThreadProc() {
    LogDbg("LoopWDog started");
    while (_wdog_state == WDogState::kRunning) {
        SendLoopFunc();

        for (int i = 0; i < 10 && _wdog_state == WDogState::kRunning; ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (_wdog_state == WDogState::kRunning)
            CheckLoopTag();
    }
    LogDbg("LoopWDog stoped");
}

//! 默认线程超时执行函数
void OnLoopBlock(const std::string &name) {
    LogWarn("loop \"%s\" block!", name.c_str());
}

//! 默认线程超时执行函数
void OnLoopRecover(const std::string &name) {
    LogNotice("loop \"%s\" recover!", name.c_str());
}

}

void LoopWDog::SetLoopBlockCallback(const LoopBlockCallback &cb) {
    TBOX_ASSERT(cb != nullptr);
    _loop_die_cb = cb;
}

void LoopWDog::Start() {
    WDogState state = WDogState::kIdle;
    if (_wdog_state.compare_exchange_strong(state, WDogState::kRunning)) {
        _sp_thread = new std::thread(ThreadProc);

    } else {
        LogWarn("it's not idle, state:%d", state);
    }
}

void LoopWDog::Stop() {
    WDogState state = WDogState::kRunning;
    if (_wdog_state.compare_exchange_strong(state, WDogState::kStoping)) {
        _sp_thread->join();
        CHECK_DELETE_RESET_OBJ(_sp_thread);
        _loop_info_vec.clear();
        _wdog_state = WDogState::kIdle;

    } else {
        LogWarn("it's not running. state:%d", state);
    }
}

void LoopWDog::Register(event::Loop *loop, const std::string &name) {
    std::lock_guard<std::mutex> lg(_mutex_lock);
    auto iter = std::find_if(_loop_info_vec.begin(), _loop_info_vec.end(),
        [loop] (const LoopInfo::SharedPtr &loop_info_sptr) {
            return loop_info_sptr->loop == loop;
        }
    );

    if (iter == _loop_info_vec.end()) { //! 如果没有找到那么创建
        _loop_info_vec.push_back(std::make_shared<LoopInfo>(loop, name));
    }
}

void LoopWDog::Unregister(event::Loop *loop) {
    std::lock_guard<std::mutex> lg(_mutex_lock);
    auto iter = std::remove_if(_loop_info_vec.begin(), _loop_info_vec.end(),
        [loop] (const LoopInfo::SharedPtr &loop_info_sptr) {
            return loop_info_sptr->loop == loop;
        }
    );

    if (iter != _loop_info_vec.end()) {
        _loop_info_vec.erase(iter, _loop_info_vec.end());
    }
}

}
}
