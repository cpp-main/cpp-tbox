#include <cassert>
#include <cstring>
#include <algorithm>
#include "fd_event.h"
#include "loop.h"

namespace tbox {
namespace event {

namespace {
short EpollEventsToLocal(short epoll_events)
{
    short ret = 0;
    if ((epoll_events & EPOLLIN) || (epoll_events & EPOLLPRI))
        ret |= FdEvent::kReadEvent;
    if (epoll_events & EPOLLOUT)
        ret |= FdEvent::kWriteEvent;

    return ret;
}

short LocalEventsToEpoll(short local_events)
{
    short ret = 0;
    if (local_events & FdEvent::kWriteEvent)
        ret |= EPOLLOUT;

    if (local_events & FdEvent::kReadEvent)
        ret |= EPOLLIN;

    return ret;
}

}

class EpollFdEventImpl
{
  public:
    explicit EpollFdEventImpl(BuiltinLoop *wp_loop)
        : wp_loop_(wp_loop)
    {
        memset(&ev_, 0, sizeof(ev_));
    }

    virtual ~EpollFdEventImpl()
    {
        disable();
    }

    bool initialize(int fd, short events)
    {
        disable();

        fd_ = fd;
        memset(&ev_, 0, sizeof(ev_));
        ev_.data.ptr = static_cast<void *>(this);
        ev_.events = LocalEventsToEpoll(events);

        is_inited_ = true;
        return true;
    }

    inline int setCallback(const FdEvent::CallbackFunc &cb)
    {
        cb_index_ ++;
        callbacks_.push_back(std::make_pair(cb_index_, cb));
        return cb_index_;
    }

    inline bool isEnabled() const
    {
        if (!is_inited_)
            return false;

        return is_enabled_;
    }

    bool enable()
    {
        if (!is_inited_)
            return false;

        if (isEnabled())
            return true;

        int ret = epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_ADD, fd_, &ev_);
        if (ret != 0)
            return false;

        is_enabled_ = true;
        return true;
    }

    bool disable()
    {
        if (!is_inited_)
            return false;

        if (!isEnabled())
            return true;

        epoll_ctl(wp_loop_->epollFd(), EPOLL_CTL_DEL, fd_, NULL);

        is_enabled_ = false;
        return true;
    }

    inline void ref() { ref_count_ ++; }
    inline bool noref() { return ref_count_ <= 0; }
    void unref(int cb_index)
    {
        -- ref_count_;
        using Element = std::pair<int, FdEvent::CallbackFunc>;
        auto is_equal_to_cb_index = [cb_index](const Element &e) -> bool
        {
            return e.first == cb_index;
        };

        auto result_it = std::find_if(callbacks_.begin(), callbacks_.end(), is_equal_to_cb_index);
        if (result_it != callbacks_.end())
            callbacks_.erase(result_it);
    }

    inline uint32_t getEvents() const
    {
        return EpollEventsToLocal(ev_.events);
    }

  public:
    static void OnEventCallback(int fd, uint32_t events, void *obj)
    {
        EpollFdEventImpl *self = static_cast<EpollFdEventImpl*>(obj);
        self->onEvent(events);
    }

  protected:
    void onEvent(uint32_t events)
    {
        short local_events = EpollEventsToLocal(events);
        for (const auto &it : callbacks_) {
            if (it.second)
                it.second(local_events);
        }
    }

  private:
    BuiltinLoop *wp_loop_;
    bool is_inited_{ false };
    bool is_enabled_{ false };

    std::vector<std::pair<int, FdEvent::CallbackFunc> > callbacks_;

    int fd_ { -1 };
    struct epoll_event ev_;

    int ref_count_{ 0 };
    int cb_index_{ 0 };
};

EpollFdEvent::EpollFdEvent(BuiltinLoop *wp_loop) :
    wp_loop_(wp_loop),
    is_stop_after_trigger_(false),
    cb_level_(0)
{
}

EpollFdEvent::~EpollFdEvent()
{
    assert(cb_level_ == 0);
    if (impl_ != nullptr) {
        impl_->unref(cb_index_);
        if (impl_->noref()) {
            wp_loop_->unregisterFdevent(fd_);
            delete impl_;
            impl_ = nullptr;
        }
    }
}

bool EpollFdEvent::initialize(int fd, short events, Mode mode)
{
    if (isEnabled())
        return false;
    fd_ = fd;
    events_ = events;
    if (mode == FdEvent::Mode::kOneshot)
        is_stop_after_trigger_ = true;

    impl_ = wp_loop_->queryFdevent(fd_);
    if (impl_ == nullptr) {
        impl_ = new EpollFdEventImpl(wp_loop_);
        assert(impl_ != nullptr);
        wp_loop_->registerFdEvent(fd, impl_);
    }

    impl_->ref();
    cb_index_ = impl_->setCallback(
            [this](short events) {
                if (events & events_)
                    onEvent(events);
            });

    return true;
}

void EpollFdEvent::setCallback(const CallbackFunc &cb)
{
    cb_ = cb;
}

bool EpollFdEvent::isEnabled() const
{
    if (nullptr == impl_)
        return false;

    if (!impl_->isEnabled())
        return false;

    auto event_bit = impl_->getEvents() & events_;
    return event_bit != 0; //!< means impl and this obj both were enabled
}

bool EpollFdEvent::enable()
{
    if (impl_ == nullptr)
        return false;

    uint32_t new_events = 0;
    if (impl_->isEnabled()) {
        uint32_t events = impl_->getEvents() & events_;
        if (events == events_)
            return true;

        impl_->disable();
        new_events = impl_->getEvents() | events_;
    } else {
        new_events = events_;
    }

    impl_->initialize(fd_, new_events);

    return impl_->enable();
}

bool EpollFdEvent::disable()
{
    if (nullptr == impl_)
        return true;

    if (!impl_->isEnabled())
        return true;

    uint32_t events = impl_->getEvents() & events_;
    if (events == 0)
        return true;

    impl_->disable();
    uint32_t new_events = impl_->getEvents() ^ events_;
    if (new_events == 0)
        return true;

    impl_->initialize(fd_, new_events);
    impl_->enable();
    return true;
}

void EpollFdEvent::OnEventCallback(int fd, uint32_t events, void *obj)
{
    EpollFdEventImpl::OnEventCallback(fd, events, obj);
}

Loop* EpollFdEvent::getLoop() const
{
    return wp_loop_;
}


void EpollFdEvent::onEvent(short events)
{
    wp_loop_->beginEventProcess();

    if (cb_) {

        ++cb_level_;
        cb_(events);
        --cb_level_;

        if (is_stop_after_trigger_)
            disable();
    }

    wp_loop_->endEventProcess();
}
}
}
