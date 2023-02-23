#include "wrapper_action.h"
#include <tbox/base/defines.h>
#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>

namespace tbox {
namespace flow {

WrapperAction::~WrapperAction() {
    CHECK_DELETE_RESET_OBJ(child_);
}

void WrapperAction::setChild(Action *child) {
    TBOX_ASSERT(child != nullptr);

    CHECK_DELETE_RESET_OBJ(child_);
    child_ = child;
    child_->setFinishCallback(
        [this](bool succ) { finish(succ); }
    );
}

void WrapperAction::toJson(Json &js) const {
    Action::toJson(js);
    child_->toJson(js["child"]);
}

bool WrapperAction::onStart() {
    if (child_ == nullptr) {
        LogWarn("no child in %s(%s)", type().c_str(), name().c_str());
        return false;
    } 
    return child_->start();
}

bool WrapperAction::onStop() {
    return child_->stop();
}

bool WrapperAction::onPause() {
    return child_->pause();
}

bool WrapperAction::onResume() {
    return child_->resume();
}

void WrapperAction::onReset() {
    child_->reset();
}

}
}
