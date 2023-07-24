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
#include <tbox/main/module.h>
#include <tbox/base/log.h>
#include <tbox/event/timer_event.h> /// 导入TimerEvent类

class MyModule : public tbox::main::Module {
  public:
    explicit MyModule(tbox::main::Context &ctx)
        : tbox::main::Module("timer_event", ctx)
        , tick_timer_(ctx.loop()->newTimerEvent())    /// 实例化定时器对象
    { }

    virtual ~MyModule() { delete tick_timer_; } /// 释放定时器对象

  public:
    virtual bool onInit(const tbox::Json &js) override {
        /// 初始化，设置定时间隔为1s，持续触发
        tick_timer_->initialize(std::chrono::seconds(1), tbox::event::Event::Mode::kPersist);
        /// 设置定时器触发时的回调
        tick_timer_->setCallback([this] { onTick(); });
        return true;
    }
    /// 启动
    virtual bool onStart() override {
        tick_timer_->enable();
        return true;
    }
    /// 停止
    virtual void onStop() override {
        tick_timer_->disable();
    }

  private:
    void onTick() {
        ++count_;
        LogDbg("count:%d", count_);
    }

  private:
    tbox::event::TimerEvent *tick_timer_;
    int count_ = 0;
};

extern "C" void RegisterApps(tbox::main::Module &apps, tbox::main::Context &ctx) {
  apps.add(new MyModule(ctx));
}
