#include "loop.h"

#include <tbox/log.h>

#include "engins/libevent/loop.h"

namespace tbox {
namespace event {

Loop* Loop::New(Engine engine)
{
    switch (engine) {
        case Engine::kLibevent:
            return new LibeventLoop;

        default:
            LogErr("Unsupport engine");
    }
    return nullptr;
}

Loop* Loop::New()
{
    return new LibeventLoop;
}

}
}
