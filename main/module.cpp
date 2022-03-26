#include "module.h"
#include <algorithm>

namespace tbox {
namespace main {

Module::Module(Context &ctx) :
    ctx_(ctx)
{ }

Module::~Module()
{
    cleanup();

    for (auto child : children_)
        delete child;
    children_.clear();
}

bool Module::addChild(Module *child)
{
    if (child == nullptr)
        return false;

    auto iter = std::find(children_.begin(), children_.end(), child);
    if (iter != children_.end())
        return false;

    children_.push_back(child);
    return true;
}

bool Module::initialize(const Json &js)
{
    if (state_ != State::kNone)
        return false;

    for (auto child : children_) {
        if (!child->initialize(js))
            return false;
    }

    state_ = State::kInited;
    return true;
}

bool Module::start()
{
    if (state_ != State::kInited)
        return false;

    for (auto child : children_) {
        if (!child->start())
            return false;
    }

    state_ = State::kRunning;
    return true;
}

void Module::stop()
{
    if (state_ != State::kRunning)
        return;

    for (auto i = children_.size() - 1; i >= 0; --i)
        children_[i]->stop();

    state_ = State::kInited;
}

void Module::cleanup()
{
    if (state_ <= State::kNone)
        return;

    for (auto i = children_.size() - 1; i >= 0; --i)
        children_[i]->cleanup();

    state_ = State::kNone;
}

}
}
