#include "apps_imp.h"

#include <cassert>
#include <vector>
#include <algorithm>

#include "app.h"

namespace tbox {
namespace main {

AppsImp::~AppsImp()
{
    cleanup();

    while (!apps_.empty()) {
        delete apps_.back();
        apps_.pop_back();
    }
}

bool AppsImp::add(App *app)
{
    if (state_ != State::kNone)
        return false;

    //! 先找找之前有没有插入过，如果有就不要再加了
    auto it = std::find(apps_.begin(), apps_.end(), app);
    if (it != apps_.end())
        return false;

    apps_.push_back(app);
    return true;
}

bool AppsImp::empty() const
{
    return apps_.empty();
}

void AppsImp::fillDefaultConfig(Json &cfg) const
{
    for (auto app : apps_)
        app->fillDefaultConfig(cfg);
}

bool AppsImp::construct(Context &ctx)
{
    if (state_ != State::kNone)
        return false;

    for (auto app : apps_) {
        if (!app->construct(ctx))
            return false;
    }

    state_ = State::kConstructed;
    return true;
}

bool AppsImp::initialize(const Json &cfg)
{
    if (state_ != State::kConstructed)
        return false;

    for (auto app : apps_) {
        if (!app->initialize(cfg))
            return false;
    }

    state_ = State::kInited;
    return true;
}

bool AppsImp::start()
{
    if (state_ != State::kInited)
        return false;

    for (auto app : apps_) {
        if (!app->start())
            return false;
    }

    state_ = State::kRunning;
    return true;
}

void AppsImp::stop()
{
    if (state_ <= State::kInited)
        return;

    for (auto rit = apps_.rbegin(); rit != apps_.rend(); ++rit)
        (*rit)->stop();

    state_ = State::kInited;
}

void AppsImp::cleanup()
{
    if (state_ <= State::kNone)
        return;

    stop();

    for (auto rit = apps_.rbegin(); rit != apps_.rend(); ++rit)
        (*rit)->cleanup();

    state_ = State::kNone;
}

}
}
