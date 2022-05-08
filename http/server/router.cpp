#include "router.h"

namespace tbox {
namespace http {
namespace server {

struct Router::Data {

};

Router::Router() :
    d_(new Data)
{ }

Router::~Router()
{
    delete d_;
}

void Router::handle(ContextSptr sp_ctx, const NextFunc &next)
{

}

Router& Router::get(const std::string &url, const RequestCallback &cb)
{
    return *this;
}

Router& Router::post(const std::string &url, const RequestCallback &cb)
{
    return *this;
}

Router& Router::put(const std::string &url, const RequestCallback &cb)
{
    return *this;
}

Router& Router::del(const std::string &url, const RequestCallback &cb)
{
    return *this;
}

}
}
}
