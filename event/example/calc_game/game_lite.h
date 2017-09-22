#ifndef CALC_GAME_H_20170922
#define CALC_GAME_H_20170922

#include <tbox/event/loop.h>
#include <tbox/event/timer_item.h>
#include <tbox/event/fd_item.h>

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

    TimerItem* sp_30sec_timer_;
    TimerItem* sp_20sec_timer_;
    TimerItem* sp_10sec_timer_;
    FdItem* sp_stdin_read_ev_;

    int right_answer_;
    int remain_question_number_;
};

#endif //CALC_GAME_H_20170922
