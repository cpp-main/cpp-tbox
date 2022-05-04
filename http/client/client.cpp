#include "client.h"

namespace tbox {
namespace http {
namespace client {

using namespace event;
using namespace network;

Client::Client(Loop *wp_loop)
{ }

Client::~Client()
{ }

bool Client::initialize(const SockAddr &server_addr)
{
    return false;
}

void Client::request(const Request &req, const RespondCallback &cb)
{ }

void Client::cleanup()
{ }

}
}
}
