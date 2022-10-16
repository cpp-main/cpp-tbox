#include "if_else_action.h"

#include <cassert>
#include <tbox/base/defines.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace action {

using namespace std::placeholders;

IfElseAction::IfElseAction(Context &ctx, const std::string &name,
    Action *cond_action, Action *if_action, Action *else_action) :
  Action(ctx, name),
  cond_action_(cond_action),
  if_action_(if_action),
  else_action_(else_action)
{
  assert(cond_action != nullptr);

  cond_action_->setFinishCallback(std::bind(&IfElseAction::onCondActionFinished, this, _1));
  if (if_action_ != nullptr)
    if_action_->setFinishCallback(std::bind(&IfElseAction::finish, this, _1));
  if (else_action_ != nullptr)
    else_action_->setFinishCallback(std::bind(&IfElseAction::finish, this, _1));
}

IfElseAction::~IfElseAction() {
  CHECK_DELETE_RESET_OBJ(cond_action_);
  CHECK_DELETE_RESET_OBJ(if_action_);
  CHECK_DELETE_RESET_OBJ(else_action_);
}

void IfElseAction::toJson(Json &js) const {
  Action::toJson(js);
  cond_action_->toJson(js["cond"]);
  if (if_action_ != nullptr)
    if_action_->toJson(js["if"]);
  if (else_action_ != nullptr)
    else_action_->toJson(js["else"]);

  js["is_cond_done"] = is_cond_done_;
  if (is_cond_done_)
    js["is_cond_succ"] = is_cond_succ_;
}

bool IfElseAction::start() {
  return Action::start() && cond_action_->start();
}

bool IfElseAction::pause() {
  if (!Action::pause())
    return false;

  if (is_cond_done_) {
    if (is_cond_succ_)
      return if_action_->pause();
    else
      return else_action_->pause();
  } else {
    return cond_action_->pause();
  }
}

bool IfElseAction::resume() {
  if (!Action::resume())
    return false;

  if (is_cond_done_) {
    if (is_cond_succ_)
      return if_action_->resume();
    else
      return else_action_->resume();
  } else {
    return cond_action_->resume();
  }
}

bool IfElseAction::stop() {
  if (!Action::stop())
    return false;

  if (is_cond_done_) {
    if (is_cond_succ_)
      return if_action_->stop();
    else
      return else_action_->stop();
  } else {
    return cond_action_->stop();
  }
}

void IfElseAction::onCondActionFinished(bool is_succ) {
  is_cond_done_ = true;
  is_cond_succ_ = is_succ;

  if (is_succ) {
    if (if_action_ != nullptr) {
      if_action_->start();
      return;
    }
  } else {
    if (else_action_ != nullptr) {
      else_action_->start();
      return;
    }
  }

  finish(true);
}

}
}
