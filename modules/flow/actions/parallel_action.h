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
#ifndef TBOX_FLOW_PARALLEL_ACTION_H_20221005
#define TBOX_FLOW_PARALLEL_ACTION_H_20221005

#include "assemble_action.h"

#include <chrono>
#include <set>

namespace tbox {
namespace flow {

/**
 * 并行动作
 */
class ParallelAction : public AssembleAction {
  public:
    //! 模式
    enum class Mode {
      kAllFinish, //!< 全部结束
      kAnyFail,   //!< 任一失败
      kAnySucc,   //!< 任一成功
    };

    explicit ParallelAction(event::Loop &loop, Mode mode = Mode::kAllFinish);
    virtual ~ParallelAction();

    virtual void toJson(Json &js) const override;

    virtual int addChild(Action *action) override;
    virtual bool isReady() const override;

    inline void setMode(Mode mode) { mode_ = mode; }

    using FinishedChildren = std::map<int, bool>;

    FinishedChildren getFinishedChildren() const { return finished_children_; }

    static std::string ToString(Mode mode);

  protected:
    virtual void onStart() override;
    virtual void onStop() override;
    virtual void onPause() override;
    virtual void onResume() override;
    virtual void onReset() override;

  private:
    void stopAllActions();
    void pauseAllActions();

    void onChildFinished(int index, bool is_succ);
    void onChildBlocked(int index, const Reason &why, const Trace &trace);

  private:
    Mode mode_;
    std::vector<Action*> children_;

    FinishedChildren finished_children_;
};

}
}

#endif //TBOX_FLOW_PARALLEL_ACTION_H_20221005
