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
#ifndef TBOX_FLOW_REPEAT_ACTION_H_20221017
#define TBOX_FLOW_REPEAT_ACTION_H_20221017

#include "../action.h"

namespace tbox {
namespace flow {

class RepeatAction : public Action {
  public:
    enum class Mode {
      kNoBreak,     //! for (int i = 0; i < times; ++i) { action() };
      kBreakFail,   //! for (int i = 0; i < times && action(); ++i);
      kBreakSucc,   //! for (int i = 0; i < times && !action(); ++i);
    };

    explicit RepeatAction(event::Loop &loop, Action *child, size_t times,
                          Mode mode = Mode::kNoBreak);
    virtual ~RepeatAction();

    virtual void toJson(Json &js) const override;

  protected:
    virtual bool onStart() override;
    virtual bool onStop() override;
    virtual bool onPause() override;
    virtual bool onResume() override;
    virtual void onReset() override;

    void onChildFinished(bool is_succ);

  private:
    Action *child_;
    size_t repeat_times_;
    size_t remain_times_ = 0;
    Mode mode_;
};

}
}

#endif //TBOX_FLOW_REPEAT_ACTION_H_20221017
