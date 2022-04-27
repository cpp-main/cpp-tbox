#include "request_pool.h"

#include <vector>
#include <tbox/base/log.h>
#include <tbox/event/timer_event.h>
#include <tbox/base/cabinet.hpp>

namespace tbox {
namespace util {

using namespace std;
using namespace event;
using namespace cabinet;

class RequestPool::Impl {
  public:
    Impl(Loop *wp_loop);
    ~Impl();

    bool initialize(const Duration &check_interval, int check_times,
                    const TimeoutAction &timeout_action);
    bool start();
    void stop();
    void cleanup();

    Token newRequest(void *req_ctx = nullptr);
    bool updateRequest(const Token &token, void *req_ctx);
    void* removeRequest(const Token &token);

  private:
    void onTimerTick();

    struct PollItem {
        PollItem *next = nullptr;
        vector<Token> tokens;
    };

    void handleRequestOnPollItem(PollItem *poll_item);

  private:
    TimerEvent   *sp_timer_;
    TimeoutAction timeout_action_;
    Cabinet<void> cabinet_;
    PollItem     *curr_item_ = nullptr;
};

RequestPool::Impl::Impl(Loop *wp_loop) :
    sp_timer_(wp_loop->newTimerEvent())
{ }

RequestPool::Impl::~Impl()
{
    delete sp_timer_;
}

bool RequestPool::Impl::initialize(const Duration &check_interval, int check_times,
                                   const TimeoutAction &timeout_action)
{
    if (check_times < 1) {
        LogWarn("check_times should >= 1");
        return false;
    }

    timeout_action_ = timeout_action;
    sp_timer_->initialize(check_interval, Event::Mode::kPersist);
    sp_timer_->setCallback(std::bind(&RequestPool::Impl::onTimerTick, this));

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

void RequestPool::Impl::cleanup()
{
    PollItem *item = curr_item_->next;
    curr_item_->next = nullptr;
    while (item != nullptr) {
        auto next = item->next;
        handleRequestOnPollItem(item);
        delete item;
        item = next;
    }

    timeout_action_ = nullptr;
}

/////////////////////////////////////
// 包装
/////////////////////////////////////

RequestPool::RequestPool(event::Loop *wp_loop) :
    impl_(new Impl(wp_loop))
{ }

RequestPool::~RequestPool()
{
    delete impl_;
}

bool RequestPool::initialize(const Duration &check_interval, int check_times,
                             const TimeoutAction &timeout_action)
{
    return impl_->initialize(check_interval, check_times, timeout_action);
}

bool RequestPool::start()
{
    return impl_->start();
}

void RequestPool::stop()
{
    impl_->stop();
}

void RequestPool::cleanup()
{
    impl_->cleanup();
}

RequestPool::Token RequestPool::newRequest(void *req_ctx)
{
    return impl_->newRequest(req_ctx);
}

bool RequestPool::updateRequest(const Token &token, void *req_ctx)
{
    return impl_->updateRequest(token, req_ctx);
}

void* RequestPool::removeRequest(const Token &token)
{
    return impl_->removeRequest(token);
}

}
}
