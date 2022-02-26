#include "loop.h"

#include <tbox/base/log.h>

#ifdef ENABLE_LIBEVENT
#include "engins/libevent/loop.h"
#endif

#ifdef ENABLE_LIBEV
#include "engins/libev/loop.h"
#endif

namespace tbox {
namespace event {

Loop* Loop::New()
{
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
