#include "loop.h"
#include <tbox/base/log.h>

#if defined(ENABLE_LIBEVENT)
#include "engins/libevent/loop.h"
#elif defined(ENABLE_LIBEV)
#include "engins/libev/loop.h"
#elif defined(ENABLE_EPOLL)
#include "engins/epoll/loop.h"
#else
#error("no engin specified!!!")
#endif

namespace tbox {
namespace event {

Loop* Loop::New(Engine engine)
{
    switch (engine) {
#if defined(ENABLE_EPOLL)
        case Engine::kEpoll:
            return new EpollLoop;
#elif defined(ENABLE_LIBEVENT)
        case Engine::kLibevent:
            return new LibeventLoop;
#elif defined(ENABLE_LIBEV)
        case Engine::kLibev:
            return new LibevLoop;
#endif
        default:
            LogErr("Unsupport engine");
    }

    return nullptr;
}

Loop* Loop::New()
{
#if defined(ENABLE_EPOLL)
    return new EpollLoop;
#elif defined(ENABLE_LIBEVENT)
    return new LibeventLoop;
#elif defined(ENABLE_LIBEV)
    return new LibevLoop;
#endif
    return NULL;
}

}
}
