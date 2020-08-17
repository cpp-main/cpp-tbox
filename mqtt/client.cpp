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

}
}
