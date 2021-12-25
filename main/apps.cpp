#include "apps.h"

#include <cassert>
#include <vector>
#include <algorithm>

#include "app.h"

namespace tbox {
namespace main {

struct Apps::Data {
    std::vector<App*> apps;
    State state = State::kNone;
};

Apps::Apps() :
    d_(new Data)
{
    assert(d_ != nullptr);
}

Apps::~Apps()
{
    cleanup();

    while (!d_->apps.empty()) {
        delete d_->apps.back();
        d_->apps.pop_back();
    }

    delete d_;
}

bool Apps::add(App *app)
{
    if (d_->state != State::kNone)
        return false;

    //! 先找找之前有没有插入过，如果有就不要再加了
    auto it = std::find(d_->apps.begin(), d_->apps.end(), app);
    if (it != d_->apps.end())
        return false;

    d_->apps.push_back(app);
    return true;
}

bool Apps::empty() const
{
    return d_->apps.empty();
}

bool Apps::initialize()
{
    if (d_->state != State::kNone)
        return false;

    for (auto app : d_->apps) {
        if (!app->initialize())
            return false;
    }

    d_->state = State::kInited;
    return true;
}

void Apps::cleanup()
{
    if (d_->state <= State::kNone)
        return;

    stop();

    for (auto rit = d_->apps.rbegin(); rit != d_->apps.rend(); ++rit)
        (*rit)->cleanup();

    d_->state = State::kNone;
}

bool Apps::start()
{
    if (d_->state != State::kInited)
        return false;

    for (auto app : d_->apps) {
        if (!app->start())
            return false;
    }

    d_->state = State::kRunning;
    return true;
}

void Apps::stop()
{
    if (d_->state <= State::kInited)
        return;

    for (auto rit = d_->apps.rbegin(); rit != d_->apps.rend(); ++rit)
        (*rit)->stop();

    d_->state = State::kInited;
}

Apps::State Apps::state() const
{
    return d_->state;
}

}
}
