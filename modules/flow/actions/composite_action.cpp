#include "composite_action.h"
#include <tbox/base/defines.h>
#include <tbox/base/assert.h>
#include <tbox/base/json.hpp>

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
    child_->toJson(js["10.child"]);
}

bool CompositeAction::onStart() {
    if (child_ == nullptr) {
        LogWarn("no child in %s(%s)", type().c_str(), name().c_str());
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
