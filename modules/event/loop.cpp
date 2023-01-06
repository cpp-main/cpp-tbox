#include "loop.h"
#include <tbox/base/log.h>

#if defined(ENABLE_LIBEVENT)
#include "engins/libevent/loop.h"
#endif

#if defined(ENABLE_LIBEV)
#include "engins/libev/loop.h"
#endif

#if defined(ENABLE_EPOLL)
#include "engins/epoll/loop.h"
#endif

namespace tbox {
namespace event {

Loop* Loop::New()
{
#ifdef ENABLE_EPOLL
    return new EpollLoop;
#endif
#ifdef ENABLE_LIBEVENT
    return new LibeventLoop;
#endif
#ifdef ENABLE_LIBEV
    return new LibevLoop;
#endif
    return nullptr;
}

Loop* Loop::New(const std::string &engine_type)
{
#ifdef ENABLE_EPOLL
    if (engine_type == "epoll")
        return new EpollLoop;
#endif
#ifdef ENABLE_LIBEVENT
    if (engine_type == "libevent")
        return new LibeventLoop;
#endif
#ifdef ENABLE_LIBEV
    if (engine_type == "libev")
        return new LibevLoop;
#endif
    return nullptr;
}

std::vector<std::string> Loop::Engines()
{
    std::vector<std::string> types;
#ifdef ENABLE_EPOLL
    types.push_back("epoll");
#endif
#ifdef ENABLE_LIBEVENT
    types.push_back("libevent");
#endif
#ifdef ENABLE_LIBEV
    types.push_back("libev");
#endif
    return types;
}

}
}
