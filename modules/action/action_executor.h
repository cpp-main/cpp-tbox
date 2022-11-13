#ifndef TBOX_ACTION_EXECUTOR_H_20221112
#define TBOX_ACTION_EXECUTOR_H_20221112

#include <deque>
#include <tbox/base/defines.h>
#include <tbox/event/loop.h>

namespace tbox {
namespace action {

class Action;

class ActionExecutor {
  public:
    virtual ~ActionExecutor();

    NONCOPYABLE(ActionExecutor);
    IMMOVABLE(ActionExecutor);

  public:
    using ActionId = int;
    using ActionCallback = std::function<void(ActionId)>;
    using Callback = std::function<void()>;

    ActionId append(Action *action, int level = 1);
    ActionId current() const;

    bool cancelCurrent();
    bool cancel(ActionId action_id);
    void stop();

    //! set callbacks
    void setActionStartedCallback(const ActionCallback &cb) { action_started_cb_ = cb; }
    void setActionFinishedCallback(const ActionCallback &cb) { action_finished_cb_ = cb; }
    void setAllFinishedCallback(const Callback &cb) { all_finished_cb_ = cb; }

  private:
    ActionId allocActionId();
    void schedule();

  private:
    struct Item {
      ActionId id;
      Action *action;
    };

    std::array<std::deque<Item>, 3> action_deque_array_;
    ActionId action_id_alloc_counter_ = 0;
    int curr_action_deque_index_ = -1;  //!< 当前正在运行中的动作所在的队列，-1表示没有动作

    ActionCallback  action_started_cb_;
    ActionCallback  action_finished_cb_;
    Callback        all_finished_cb_;
};

}
}

#endif //TBOX_ACTION_EXECUTOR_H_20221112
