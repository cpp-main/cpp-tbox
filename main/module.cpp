#include "module.h"
#include <algorithm>
#include <tbox/base/json.hpp>

namespace tbox {
namespace main {

Module::Module(const std::string &name, Context &ctx) :
    name_(name), ctx_(ctx)
{ }

Module::~Module()
{
    cleanup();

    for (const auto &item : children_)
        delete item.module_ptr;
    children_.clear();
}

bool Module::addChild(Module *child, bool required)
{
    if (state_ != State::kNone)
        return false;

    if (child == nullptr)
        return false;

    auto iter = std::find_if(children_.begin(), children_.end(),
        [child] (const ModuleItem &item) {
            return item.module_ptr == child;
        }
    );
    if (iter != children_.end())
        return false;

    children_.emplace_back(ModuleItem{ child, required });
    return true;
}

bool Module::initialize(const Json &js_parent)
{
    if (state_ != State::kNone)
        return false;

    if (!js_parent.contains(name_))
        return false;

    const Json &js_this = js_parent[name_];

    if (!onInitialize(js_this))
        return false;

    for (const auto &item : children_) {
        if (!item.module_ptr->initialize(js_this) && item.required)
            return false;
    }

    state_ = State::kInited;
    return true;
}

bool Module::start()
{
    if (state_ != State::kInited)
        return false;

    if (!onStart())
        return false;

    for (const auto &item : children_) {
        if (!item.module_ptr->start() && item.required)
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
        children_[i].module_ptr->stop();

    onStop();

    state_ = State::kInited;
}

void Module::cleanup()
{
    if (state_ == State::kNone)
        return;

    for (auto i = children_.size() - 1; i >= 0; --i)
        children_[i].module_ptr->cleanup();

    onCleanup();

    state_ = State::kNone;
}

}
}
