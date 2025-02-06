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
#ifndef TBOX_FLOW_LOOP_ACTION_H_20221017
#define TBOX_FLOW_LOOP_ACTION_H_20221017

#include "assemble_action.h"

namespace tbox {
namespace flow {

class LoopAction : public SerialAssembleAction {
  public:
    enum class Mode {
      kForever,     //! while(true) { action() };
      kUntilFail,   //! while(action());
      kUntilSucc,   //! while(!action());
    };

    explicit LoopAction(event::Loop &loop, Mode mode = Mode::kForever);
    explicit LoopAction(event::Loop &loop, Action *child, Mode mode = Mode::kForever);
    virtual ~LoopAction();

    virtual void toJson(Json &js) const override;
    virtual bool setChild(Action *child) override;
    virtual bool isReady() const override;

    inline void setMode(Mode mode) { mode_ = mode; }

  protected:
    virtual void onStart() override;
    virtual void onReset() override;

    void onChildFinished(bool is_succ, const Reason &why, const Trace &trace);

  private:
    Action *child_ = nullptr;
    Mode mode_;
};

}
}

#endif //TBOX_FLOW_LOOP_ACTION_H_20221017
