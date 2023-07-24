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

#include "../action.h"

namespace tbox {
namespace flow {

/**
 * 模拟实现以下流程
 * bool SequenceAction(action_vec, finish_condition == AllFinish) {
 *   for (item : action_vec) {
 *     auto is_succ = item();
 *     if ((finish_condition == AnySucc && is_succ) ||
 *         (finish_condition == AnyFail && !is_succ))
 *       return is_succ;
 *   }
 *   return true;
 * }
 */
class SequenceAction : public Action {
  public:
    //! 结束条件
    enum class FinishCondition {
      kAllFinish, //!< 全部结束
      kAnyFail,   //!< 任一失败
      kAnySucc,   //!< 任一成功
    };

  public:
    explicit SequenceAction(event::Loop &loop);
    virtual ~SequenceAction();

    virtual void toJson(Json &js) const override;

    int append(Action *action);

    inline void setFinishCondition(FinishCondition finish_condition) {
      finish_condition_ = finish_condition;
    }

    inline int index() const { return index_; }

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

  private:
    void startOtheriseFinish(bool is_succ);
    void onChildFinished(bool is_succ);

  private:
    FinishCondition finish_condition_ = FinishCondition::kAllFinish;
    size_t index_ = 0;
    std::vector<Action*> children_;
};

}
}

#endif //TBOX_FLOW_SEQUENCE_ACTION_H_20221002
