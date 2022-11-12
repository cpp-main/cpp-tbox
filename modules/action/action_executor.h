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

    ActionId append(Action *action);
    ActionId current() const;

    bool skip();
    bool cancel(ActionId action_id);
    size_t size() const;

    //! set callbacks
    void setActionStartedCallback(const ActionCallback &cb) { action_started_cb_ = cb; }
    void setActionFinishedCallback(const ActionCallback &cb) { action_finished_cb_ = cb; }
    void setAllFinishedCallback(const Callback &cb) { all_finished_cb_ = cb; }

  private:
    ActionId allocActionId();
    void run();

  private:
    struct Item {
      ActionId id;
      Action *action;
    };

    std::deque<Item> action_deque_;
    ActionId action_id_alloc_counter_ = 0;

    ActionCallback  action_started_cb_;
    ActionCallback  action_finished_cb_;
    Callback        all_finished_cb_;
};

}
}

#endif //TBOX_ACTION_EXECUTOR_H_20221112
