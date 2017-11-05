#ifndef CALC_GAME_H_201709211
#define CALC_GAME_H_201709211

#include <tbox/event/loop.h>
#include <tbox/event/timer_event.h>
#include <tbox/event/fd_event.h>

using namespace tbox::event;

class Game {
  public:
    Game();
    ~Game();

    void init(Loop *wp_loop);
    void cleanup();

  protected:
    void onStartGame();
    void askQuestion();
    void on30SecReach();
    void on20SecReach();
    void on10SecReach();
    void onStdinReadable(short event);

  private:
    Loop *wp_loop_;

    TimerEvent* sp_start_timer_;
    TimerEvent* sp_30sec_timer_;
    TimerEvent* sp_20sec_timer_;
    TimerEvent* sp_10sec_timer_;
    FdEvent* sp_stdin_read_ev_;

    int start_countdown_;
    int right_answer_;
    int remain_question_number_;
    time_t start_tstamp_;
    bool user_start_;
};

#endif //CALC_GAME_H_201709211
