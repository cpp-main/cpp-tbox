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

#include <limits>
#include "assemble_action.h"

namespace tbox {
namespace flow {

class RepeatAction : public SerialAssembleAction {
  public:
    enum class Mode {
      kNoBreak,     //! for (int i = 0; i < times; ++i) { action() };
      kBreakFail,   //! for (int i = 0; i < times && action(); ++i);
      kBreakSucc,   //! for (int i = 0; i < times && !action(); ++i);
    };

    explicit RepeatAction(event::Loop &loop);
    explicit RepeatAction(event::Loop &loop, size_t times, Mode mode = Mode::kNoBreak);
    explicit RepeatAction(event::Loop &loop, Action *child, size_t times, Mode mode = Mode::kNoBreak);
    virtual ~RepeatAction();

    virtual void toJson(Json &js) const override;
    virtual bool setChild(Action *action) override;
    virtual bool isReady() const override;

    inline void setMode(Mode mode) { mode_ = mode; }
    inline void setTimes(size_t times) { repeat_times_ = times; }

  protected:
    virtual void onStart() override;
    virtual void onReset() override;

    void onChildFinished(bool is_succ, const Reason &why, const Trace &trace);

  private:
    Mode mode_ = Mode::kNoBreak;
    size_t repeat_times_ = std::numeric_limits<size_t>::max();
    Action *child_ = nullptr;

    size_t remain_times_ = 0;
};

}
}

#endif //TBOX_FLOW_REPEAT_ACTION_H_20221017
