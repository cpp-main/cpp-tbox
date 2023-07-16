#ifndef TBOX_EVENT_EPOLL_TYPES_H_20230716
#define TBOX_EVENT_EPOLL_TYPES_H_20230716

#include <sys/epoll.h>
#include <vector>

namespace tbox {
namespace event {

class EpollFdEvent;

//! 同一个fd共享的数据
struct EpollFdSharedData {
    int ref = 0;    //! 引用计数
    struct epoll_event ev;
    std::vector<EpollFdEvent*> read_events;
    std::vector<EpollFdEvent*> write_events;
};

}
}

#endif //TBOX_EVENT_EPOLL_TYPES_H_20230716
