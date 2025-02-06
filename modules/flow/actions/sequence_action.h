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
#ifndef TBOX_FLOW_SEQUENCE_ACTION_H_20221002
#define TBOX_FLOW_SEQUENCE_ACTION_H_20221002

#include "assemble_action.h"

namespace tbox {
namespace flow {

/**
 * 顺序动作
 *
 * 模拟实现以下流程
 * bool SequenceAction(action_vec, mode == AllFinish) {
 *   for (item : action_vec) {
 *     auto is_succ = item();
 *     if ((mode == AnySucc && is_succ) ||
 *         (mode == AnyFail && !is_succ))
 *       return is_succ;
 *   }
 *   return true;
 * }
 */
class SequenceAction : public SerialAssembleAction {
  public:
    //! 模式
    enum class Mode {
      kAllFinish, //!< 全部结束
      kAnyFail,   //!< 任一失败
      kAnySucc,   //!< 任一成功
    };

  public:
    explicit SequenceAction(event::Loop &loop, Mode mode = Mode::kAllFinish);
    virtual ~SequenceAction();

    virtual void toJson(Json &js) const override;
    virtual int addChild(Action *action) override;
    virtual bool isReady() const override;

    inline void setMode(Mode mode) { mode_ = mode; }
    inline int index() const { return index_; }

    static std::string ToString(Mode mode);

  protected:
    virtual void onStart() override;
    virtual void onReset() override;

  private:
    void startOtheriseFinish(bool is_succ, const Reason &reason, const Trace &trace);
    void onChildFinished(bool is_succ, const Reason &reason, const Trace &trace);

  private:
    Mode mode_;
    size_t index_ = 0;
    std::vector<Action*> children_;
};

}
}

#endif //TBOX_FLOW_SEQUENCE_ACTION_H_20221002
