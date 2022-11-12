#include "action_executor.h"

#include <algorithm>
#include "action.h"

namespace tbox {
namespace action {

ActionExecutor::~ActionExecutor() {
  for (auto item : action_deque_)
        delete item.action;
    action_deque_.clear();
}

ActionExecutor::ActionId ActionExecutor::append(Action *action) {
  Item item = {
    .id = allocActionId(),
    .action = action
  };
  action_deque_.push_back(item);
  action->setFinishCallback([this](bool) { run(); });
  run();
  return item.id;
}

ActionExecutor::ActionId ActionExecutor::current() const {
  if (action_deque_.empty())
    return -1;
  return action_deque_.front().id;
}

bool ActionExecutor::skip() {
  if (action_deque_.empty())
    return false;

  auto item = action_deque_.front();
  item.action->stop();
  delete item.action;
  action_deque_.pop_front();
  run();
  return true;
}

bool ActionExecutor::cancel(ActionId action_id) {
  auto iter = std::find_if(action_deque_.begin(), action_deque_.end(),
    [action_id] (const Item &item) { return item.id == action_id; }
  );

  if (iter != action_deque_.end()) {
    delete iter->action;
    action_deque_.erase(iter);
    run();
    return true;
  }
  return false;
}

size_t ActionExecutor::size() const {
  return action_deque_.size();
}

ActionExecutor::ActionId ActionExecutor::allocActionId() { return ++action_id_alloc_counter_; }

void ActionExecutor::run() {
  while (!action_deque_.empty()) {
    auto item = action_deque_.front();

    if (item.action->state() == Action::State::kIdle) {
      item.action->start();
      if (action_started_cb_)
        action_started_cb_(item.id);

    } else if (item.action->state() == Action::State::kFinished) {
      action_deque_.pop_front();
      delete item.action;
      if (action_finished_cb_)
        action_finished_cb_(item.id);

    } else {
      break;
    }
  }

  if (action_deque_.empty()) {
    if (all_finished_cb_)
      all_finished_cb_();
  }
}

}
}
