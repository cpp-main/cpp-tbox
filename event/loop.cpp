#include "loop.h"

#include <tbox/base/log.h>

#ifdef ENABLE_LIBEVENT
#include "engins/libevent/loop.h"
#endif

#ifdef ENABLE_LIBEV
#include "engins/libev/loop.h"
#endif

#ifdef ENABLE_EPOLL
#include "engins/epoll/loop.h"
#endif

namespace tbox {
namespace event {

Loop* Loop::New(Engine engine)
{
    switch (engine) {
#ifdef ENABLE_LIBEVENT
        case Engine::kLibevent:
            return new LibeventLoop;
#endif
#ifdef ENABLE_LIBEV
        case Engine::kLibev:
            return new LibevLoop;
#endif

#ifdef ENABLE_EPOLL
        case Engine::kEpoll:
            return new EpollLoop;
#endif

        default:
            LogErr("Unsupport engine");
    }
    return nullptr;
}

Loop* Loop::New()
{
#ifdef ENABLE_LIBEVENT
    return new LibeventLoop;
#endif
#ifdef ENABLE_LIBEV
    return new LibevLoop;
#endif
#ifdef ENABLE_EPOLL
    return new EpollLoop;
#endif
    return NULL;
}

}
}
