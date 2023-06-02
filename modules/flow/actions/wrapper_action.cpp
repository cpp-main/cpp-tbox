#include "wrapper_action.h"
#include <base/defines.h>
#include <base/assert.h>
#include <base/json.hpp>

namespace tbox {
namespace flow {

WrapperAction::WrapperAction(event::Loop &loop, Action *child, Mode mode) :
    Action(loop, "Wrapper"),
    child_(child),
    mode_(mode)
{
    TBOX_ASSERT(child_ != nullptr);

    if (mode == Mode::kNormal)
        child_->setFinishCallback([this](bool succ) { finish(succ); });
    else if (mode == Mode::kInvert)
        child_->setFinishCallback([this](bool succ) { finish(!succ); });
    else if (mode == Mode::kAlwaySucc)
        child_->setFinishCallback([this](bool) { finish(true); });
    else if (mode == Mode::kAlwayFail)
        child_->setFinishCallback([this](bool) { finish(false); });
    else
        TBOX_ASSERT(false);
}

WrapperAction::~WrapperAction() {
    CHECK_DELETE_RESET_OBJ(child_);
}

void WrapperAction::toJson(Json &js) const {
    Action::toJson(js);
    js["mode"] = ToString(mode_);
    child_->toJson(js["child"]);
}

bool WrapperAction::onStart() {
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

std::string ToString(WrapperAction::Mode mode) {
    const char *tbl[] = {"Normal", "Invert", "AlwaySucc", "AlwayFail"};
    auto idx = static_cast<size_t>(mode); // size_t id always >= 0
    if (idx < NUMBER_OF_ARRAY(tbl))
        return tbl[idx];
    else
        return "Unknown";
}

}
}
