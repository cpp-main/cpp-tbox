#include "composite_action.h"
#include <base/defines.h>
#include <base/assert.h>
#include <base/json.hpp>

namespace tbox {
namespace flow {

CompositeAction::~CompositeAction() {
    CHECK_DELETE_RESET_OBJ(child_);
}

void CompositeAction::setChild(Action *child) {
    TBOX_ASSERT(child != nullptr);

    CHECK_DELETE_RESET_OBJ(child_);
    child_ = child;
    child_->setFinishCallback(
        [this](bool succ) { finish(succ); }
    );
}

void CompositeAction::toJson(Json &js) const {
    Action::toJson(js);
    child_->toJson(js["child"]);
}

bool CompositeAction::onStart() {
    if (child_ == nullptr) {
        LogWarn("no child in %d:%s(%s)", id(), type().c_str(), label().c_str());
        return false;
    }
    return child_->start();
}

bool CompositeAction::onStop() {
    return child_->stop();
}

bool CompositeAction::onPause() {
    return child_->pause();
}

bool CompositeAction::onResume() {
    return child_->resume();
}

void CompositeAction::onReset() {
    child_->reset();
}

}
}
