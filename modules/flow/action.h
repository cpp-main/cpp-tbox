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
#ifndef TBOX_FLOW_FLOW_H_20221001
#define TBOX_FLOW_FLOW_H_20221001

#include <string>
#include <functional>
#include <chrono>
#include <memory>

#include <tbox/base/defines.h>
#include <tbox/base/json_fwd.h>
#include <tbox/event/loop.h>
#include <tbox/util/variables.h>

#include "action_reason.h"

namespace tbox {
namespace flow {

//! 动作类
class Action {
  public:
    explicit Action(event::Loop &loop, const std::string &type);
    virtual ~Action();

    NONCOPYABLE(Action);
    IMMOVABLE(Action);

  public:
    //! 状态
    enum class State {
      kIdle,      //!< 未启动状态
      kRunning,   //!< 正在运行状态
      kPause,     //!< 暂停状态
      kFinished,  //!< 结束状态，自主通过 finish() 结束
      kStoped     //!< 停止状态，外部通过 stop() 结束
    };

    //! 结果
    enum class Result {
      kUnsure,    //!< 未知
      kSuccess,   //!< 成功
      kFail,      //!< 失败
    };

    //! 失败或阻塞原因
    struct Reason {
      int code;             //!< 错误码
      std::string message;  //!< 错误说明

      Reason() : code(0) { }
      Reason(int c) : code(c) { }
      Reason(const std::string &m) : message(m) { }
      Reason(int c, const std::string &m) : code(c), message(m) { }

      Reason(const Reason &other);
      Reason& operator = (const Reason &other);
    };

    struct Who {
      int id;
      std::string type;
      std::string label;

      Who() : id(0) { }
      Who(int i, const std::string &t, const std::string &l) : id(i), type(t), label(l) { }
      Who(const Who &other);
      Who& operator = (const Who &other);
    };
    using Trace = std::vector<Who>;

    inline int id() const { return id_; }
    inline const std::string& type() const { return type_; }

    inline State state() const { return state_; }
    inline Result result() const { return result_; }

    /// 是否正在运行
    inline bool isRunning() const { return state_ == State::kRunning; }

    /// 表示已启动，但还没有结束或终止的状态
    inline bool isUnderway() const { return state_ == State::kRunning || state_ == State::kPause; }

    inline void set_label(const std::string &label) { label_ = label; }
    inline const std::string& label() const { return label_; }

    //!< 设置结束回调
    using FinishCallback = std::function<void(bool is_succ, const Reason &why, const Trace &trace)>;
    inline void setFinishCallback(FinishCallback &&cb) { finish_cb_ = std::move(cb); }

    //!< 设置阻塞回调
    using BlockCallback = std::function<void(const Reason &, const Trace &trace)>;
    inline void setBlockCallback(BlockCallback &&cb) { block_cb_ = std::move(cb); }

    //!< 设置与取消超时
    void setTimeout(std::chrono::milliseconds ms);
    void resetTimeout();

    virtual void toJson(Json &js) const;

    virtual bool isReady() const = 0;

    bool start();   //!< 开始
    bool pause();   //!< 暂停
    bool resume();  //!< 恢复
    bool stop();    //!< 停止
    void reset();   //!< 重置，将所有的状态恢复到刚构建状态

    bool setParent(Action *parent);

    util::Variables& vars() { return vars_; }

  protected:
    //! 主动暂停动作
    bool block(const Reason &why = Reason(),    //!< 阻塞原因
               const Trace &trace = Trace());   //!< 谁结束的

    //! 主动结束动作
    bool finish(bool is_succ,                   //!< 是否成功
                const Reason &why = Reason(),   //!< 结束原因
                const Trace &trace = Trace());  //!< 谁结束的

    virtual void onStart();
    virtual void onPause();
    virtual void onBlock(const Reason &why, const Trace &trace);
    virtual void onResume();
    virtual void onStop();
    virtual void onReset();
    virtual void onFinished(bool is_succ, const Reason &why, const Trace &trace);
    virtual void onFinal() { }
    virtual void onTimeout();

    void cancelDispatchedCallback();

  protected:
    event::Loop &loop_;

  private:
    static int _id_alloc_counter_;

    int id_;
    std::string type_;
    std::string label_;
    FinishCallback  finish_cb_;
    BlockCallback   block_cb_;

    State state_ = State::kIdle;      //!< 状态
    Result result_ = Result::kUnsure; //!< 运行结果

    event::TimerEvent *timer_ev_ = nullptr;

    //! runNext()的任务号，用于撤消
    event::Loop::RunId finish_cb_run_id_ = 0;
    event::Loop::RunId block_cb_run_id_ = 0;

    bool is_base_func_invoked_ = false; //! 是否已调用基类函数
    //! 检查使用者在重写的 onStart(),onPause(),onResume(),onStop(),onFinished() 中是否调用了基类的函数
    //! 如果没有调用，则打警告提示

    Action *parent_ = nullptr;
    util::Variables vars_;
};

//! 枚举转字串
std::string ToString(Action::State state);
std::string ToString(Action::Result result);
std::string ToString(const Action::Reason &reason);
std::string ToString(const Action::Trace &trace);

}
}

#endif //TBOX_FLOW_FLOW_H_20221001
