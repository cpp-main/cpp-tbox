#include "timeout_monitor.h"

#include <vector>
#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/event/timer_event.h>

namespace tbox {
namespace eventx {

using namespace std;
using namespace event;
using namespace cabinet;

class TimeoutMonitor::Impl {
  public:
    Impl(Loop *wp_loop);
    ~Impl();

    bool initialize(const Duration &check_interval, int check_times);
    void setCallback(const Callback &cb) { cb_ = cb; }
    void add(const Token &token);
    void cleanup();

  private:
    void onTimerTick();

    struct PollItem {
        PollItem *next = nullptr;
        vector<Token> tokens;
    };

  private:
    TimerEvent *sp_timer_;
    Callback    cb_;
    int         cb_level_ = 0;
    PollItem   *curr_item_ = nullptr;
    int         token_number_ = 0;
};

TimeoutMonitor::Impl::Impl(Loop *wp_loop) :
    sp_timer_(wp_loop->newTimerEvent())
{ }

TimeoutMonitor::Impl::~Impl()
{
    TBOX_ASSERT(cb_level_ == 0);
    cleanup();
    delete sp_timer_;
}

bool TimeoutMonitor::Impl::initialize(const Duration &check_interval, int check_times)
{
    if (check_times < 1) {
        LogWarn("check_times should >= 1");
        return false;
    }

    sp_timer_->initialize(check_interval, Event::Mode::kPersist);
    sp_timer_->setCallback(std::bind(&TimeoutMonitor::Impl::onTimerTick, this));

    //! 创建计时环
    curr_item_ = new PollItem;
    auto last_item = curr_item_;
    for (int i = 1; i < check_times; ++i) {
        auto new_item = new PollItem;
        last_item->next = new_item;
        last_item = new_item;
    }
    last_item->next = curr_item_;

    return true;
}

void TimeoutMonitor::Impl::add(const Token &token)
{
    curr_item_->tokens.push_back(token);
    if (token_number_ == 0)
        sp_timer_->enable();
    ++token_number_;
}

void TimeoutMonitor::Impl::cleanup()
{
    if (curr_item_ == nullptr)
        return;

    if (token_number_ > 0)
        sp_timer_->disable();
    token_number_ = 0;

    PollItem *item = curr_item_->next;
    curr_item_->next = nullptr;
    curr_item_ = nullptr;

    while (item != nullptr) {
        auto next = item->next;
        delete item;
        item = next;
    }

    cb_ = nullptr;
}

void TimeoutMonitor::Impl::onTimerTick()
{
    curr_item_ = curr_item_->next;

    vector<Token> tobe_handle;
    swap(tobe_handle, curr_item_->tokens);

    token_number_ -= tobe_handle.size();
    if (token_number_ == 0)
        sp_timer_->disable();

    if (cb_) {
        ++cb_level_;
        for (auto token : tobe_handle)
            cb_(token);
        --cb_level_;
    }
}

/////////////////////////////////////
// 包装
/////////////////////////////////////

TimeoutMonitor::TimeoutMonitor(event::Loop *wp_loop) :
    impl_(new Impl(wp_loop))
{ }

TimeoutMonitor::~TimeoutMonitor()
{
    delete impl_;
}

bool TimeoutMonitor::initialize(const Duration &check_interval, int check_times)
{
    return impl_->initialize(check_interval, check_times);
}

void TimeoutMonitor::setCallback(const Callback &cb)
{
    impl_->setCallback(cb);
}

void TimeoutMonitor::add(const Token &token)
{
    impl_->add(token);
}

void TimeoutMonitor::cleanup()
{
    impl_->cleanup();
}

}
}
