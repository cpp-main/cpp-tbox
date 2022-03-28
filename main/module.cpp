#include "module.h"
#include <algorithm>

namespace tbox {
namespace main {

Module::Module(Context &ctx) :
    ctx_(ctx)
{ }

Module::~Module()
{
    for (const auto &item : children_)
        delete item.module_ptr;
    children_.clear();
}

bool Module::addChild(Module *child, bool required)
{
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

bool Module::initialize(const Json &js)
{
    for (const auto &item : children_) {
        if (!item.module_ptr->initialize(js) && item.required)
            return false;
    }
    return true;
}

bool Module::start()
{
    for (const auto &item : children_) {
        if (!item.module_ptr->start() && item.required)
            return false;
    }
    return true;
}

void Module::stop()
{
    for (auto i = children_.size() - 1; i >= 0; --i)
        children_[i].module_ptr->stop();
}

void Module::cleanup()
{
    for (auto i = children_.size() - 1; i >= 0; --i)
        children_[i].module_ptr->cleanup();
}

}
}
