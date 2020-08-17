#include "client.h"
#include <tbox/base/log.h>

namespace tbox {
namespace mqtt {

struct Client::Data {
};

Client::Client(event::Loop *wp_loop)
{
    LogUndo();
}

Client::~Client()
{
    LogUndo();
}

bool Client::initialize(const Config &config)
{
    LogUndo();
    return false;
}

void Client::cleanup()
{
    LogUndo();
}

void Client::setCallbacks(const Callbacks &cbs)
{
    LogUndo();
}

bool Client::start()
{
    LogUndo();
    return false;
}

bool Client::stop()
{
    LogUndo();
    return false;
}

int Client::subscribe(const std::string &topic, int *mid, int qos)
{
    LogUndo();
    return 0;
}

int Client::ubsubscribe(const std::string &topic, int *mid)
{
    LogUndo();
    return 0;
}

int Client::publish(const std::string &topic, const void *payload_ptr, size_t payload_size,
                    int qos, bool retain, int *mid)
{
    LogUndo();
    return 0;
}

bool Client::isConnected() const
{
    LogUndo();
    return false;
}

}
}
