#include "module.h"
#include <algorithm>

namespace tbox {
namespace main {

Module::Module(Context &ctx) :
    ctx_(ctx)
{ }

Module::~Module()
{
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
    for (auto child : children_) {
        if (!child->initialize(js))
            return false;
    }
    return true;
}

bool Module::start()
{
    for (auto child : children_) {
        if (!child->start())
            return false;
    }
    return true;
}

void Module::stop()
{
    for (auto i = children_.size() - 1; i >= 0; --i)
        children_[i]->stop();
}

void Module::cleanup()
{
    for (auto i = children_.size() - 1; i >= 0; --i)
        children_[i]->cleanup();
}

}
}
