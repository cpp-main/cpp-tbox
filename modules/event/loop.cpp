#include "loop.h"
#include <base/log.h>

#include "engins/epoll/loop.h"

namespace tbox {
namespace event {

Loop* Loop::New()
{
    return new EpollLoop;
}

Loop* Loop::New(const std::string &engine_type)
{
    if (engine_type == "epoll")
        return new EpollLoop;

    return nullptr;
}

std::vector<std::string> Loop::Engines()
{
    std::vector<std::string> types;

    types.push_back("epoll");
    return types;
}

}
}
