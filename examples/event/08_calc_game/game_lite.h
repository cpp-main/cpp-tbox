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
#ifndef CALC_GAME_H_20170922
#define CALC_GAME_H_20170922

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/event/fd_event.h>

using namespace tbox::event;

class GameLite {
  public:
    GameLite();
    ~GameLite();

    void init(Loop *wp_loop);
    void cleanup();

  protected:
    void askQuestion();
    void on30SecReach();
    void on20SecReach();
    void on10SecReach();
    void onStdinReadable(short event);

  private:
    Loop *wp_loop_;

    TimerEvent* sp_30sec_timer_;
    TimerEvent* sp_20sec_timer_;
    TimerEvent* sp_10sec_timer_;
    FdEvent* sp_stdin_read_ev_;

    int right_answer_;
    int remain_question_number_;
};

#endif //CALC_GAME_H_20170922
