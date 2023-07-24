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

#include "loop_wdog.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/defines.h>

namespace tbox {
namespace eventx {

namespace {

void OnLoopBlock(const std::string &name);
void OnLoopRecover(const std::string &name);

enum class State {
    kTobeCheck, //! 未打卡
    kChecked,   //! 已打卡
    kTimeout    //! 打卡超时
};

struct LoopInfo {
    using SharedPtr = std::shared_ptr<LoopInfo>;

    LoopInfo(event::Loop *l, const std::string &n) :
        loop(l), name(n),
        state(State::kChecked)
    { }

    event::Loop*  loop;
    std::string   name;
    State         state;
};

using LoopInfoVec = std::vector<LoopInfo::SharedPtr>;

LoopInfoVec     _loop_info_vec; //! 线程信息表
std::mutex      _mutex_lock;    //! 锁
std::thread*    _sp_thread = nullptr; //! 线程对象
bool _keep_running = false;     //! 线程是否继续工作标记

LoopWDog::LoopBlockCallback _loop_die_cb = OnLoopBlock;  //! 回调函数
LoopWDog::LoopBlockCallback _loop_recover_cb = OnLoopRecover;  //! 回调函数

void SendLoopFunc() {
    std::lock_guard<std::mutex> lg(_mutex_lock);
    for (auto loop_info_sptr : _loop_info_vec) {
        if (loop_info_sptr->loop->isRunning()) {
            if (loop_info_sptr->state == State::kChecked) {
                loop_info_sptr->state = State::kTobeCheck;
                loop_info_sptr->loop->runInLoop(
                    [loop_info_sptr] {
                        if (loop_info_sptr->state == State::kTimeout)
                            _loop_recover_cb(loop_info_sptr->name);
                        loop_info_sptr->state = State::kChecked;
                    },
                    "LoopWdog"
                );
            }
        }
    }
}

void CheckLoopTag() {
    std::lock_guard<std::mutex> lg(_mutex_lock);
    for (auto loop_info_sptr: _loop_info_vec) {
        if (loop_info_sptr->state == State::kTobeCheck) {
            loop_info_sptr->state = State::kTimeout;
            _loop_die_cb(loop_info_sptr->name);
        }
    }
}

//! 监控线程函数
void ThreadProc() {
    while (_keep_running) {
        SendLoopFunc();

        for (int i = 0; i < 10 && _keep_running; ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

        if (_keep_running)
            CheckLoopTag();
    }
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
    std::lock_guard<std::mutex> lg(_mutex_lock);
    if (!_keep_running) {
        _keep_running = true;
        _sp_thread = new std::thread(ThreadProc);
    }
}

void LoopWDog::Stop() {
    std::lock_guard<std::mutex> lg(_mutex_lock);
    if (_keep_running) {
        _keep_running = false;
        _sp_thread->join();
        CHECK_DELETE_RESET_OBJ(_sp_thread);
        _loop_info_vec.clear();
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
