#include "loop.h"
#include <tbox/base/log.h>

#if defined(ENABLE_LIBEVENT)
#include "engins/libevent/loop.h"
#elif defined(ENABLE_LIBEV)
#include "engins/libev/loop.h"
#elif defined(ENABLE_BUILTIN)
#include "engins/builtin/loop.h"
#else
#error("no engin specified!!!")
#endif

namespace tbox {
namespace event {

Loop* Loop::New(Engine engine)
{
    switch (engine) {
#if defined(ENABLE_LIBEVENT)
        case Engine::kLibevent:
            return new LibeventLoop;
#elif defined(ENABLE_LIBEV)
        case Engine::kLibev:
            return new LibevLoop;
#elif defined(ENABLE_BUILTIN)
        case Engine::kEpoll:
            return new BuiltinLoop;
#endif

        default:
            LogErr("Unsupport engine");
    }

    return nullptr;
}

Loop* Loop::New()
{
#if defined(ENABLE_LIBEVENT)
    return new LibeventLoop;
#elif defined(ENABLE_LIBEV)
    return new LibevLoop;
#elif defined(ENABLE_BUILTIN)
    return new BuiltinLoop;
#endif
    return NULL;
}

}
}
