#include "context.h"
#include "server.h"

namespace tbox {
namespace http {

Context::Context(Server::Impl *wp_server, const ConnToken &ct, int req_index, Request *req) :
    wp_server_(wp_server),
    conn_token_(ct),
    req_index_(req_index),
    req_(req),
    res_(new Respond)
{ }

Context::~Context()
{
    if (!is_done_)
        done();

    CHECK_DELETE_RESET_OBJ(sp_req_);
    CHECK_DELETE_RESET_OBJ(sp_res_);
}

bool Context::done()
{
    if (is_done_)
        return false;

    wp_server_->commitRespond(conn_token_, req_index_, sp_res_->toString());
}

}
}
