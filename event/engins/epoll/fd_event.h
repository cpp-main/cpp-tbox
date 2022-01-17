#ifndef TBOX_EVENT_LIBEPOLL_FD_EVENT_H_20220110
#define TBOX_EVENT_LIBEPOLL_FD_EVENT_H_20220110

#include "../../fd_event.h"

namespace tbox {
namespace event {

class EpollLoop;
struct EventData;

class EpollFdEvent : public FdEvent {
  public:
    explicit EpollFdEvent(EpollLoop *wp_loop);
    virtual ~EpollFdEvent();

  public:
    virtual bool initialize(int fd, short events, Mode mode);
    virtual void setCallback(const CallbackFunc &cb);

    virtual bool isEnabled() const;
    virtual bool enable();
    virtual bool disable();

    virtual Loop* getLoop() const;

  protected:
    static void HandleEvent(int fd, uint32_t events, void *obj);
    void onEvent(short events);

  private:
    EpollLoop *wp_loop_;
    bool is_inited_{ false };
    bool is_enabled_{ false };
    bool is_stop_after_trigger_ { false };
    CallbackFunc cb_;
    int cb_level_{ 0 };
    EventData *fd_event_data_{ nullptr };
};

}
}

#endif //TBOX_EVENT_LIBEPOLL_FD_EVENT_H_20220110
