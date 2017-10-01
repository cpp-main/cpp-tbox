#include "loop.h"

#include <tbox/log.h>

#ifdef ENABLE_LIBEVENT
#include "engins/libevent/loop.h"
#endif

#ifdef ENABLE_LIBEV
#include "engins/libev/loop.h"
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
    return NULL;
}

}
}
