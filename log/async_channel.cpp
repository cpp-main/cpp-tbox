#include "async_channel.h"

namespace tbox {
namespace log {

AsyncChannel::AsyncChannel()
{ }

AsyncChannel::~AsyncChannel()
{ }

bool AsyncChannel::initialize(const Config &cfg)
{
    return true;
}

void AsyncChannel::onLogFrontEnd(LogContent *content)
{
    //!TODO
}

}
}
