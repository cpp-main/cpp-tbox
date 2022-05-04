#include "context.h"
#include "server_imp.h"

namespace tbox {
namespace http {
namespace server {

Context::Context(Server::Impl *wp_server, const cabinet::Token &ct,
                 int req_index, Request *req) :
    wp_server_(wp_server),
    conn_token_(ct),
    req_index_(req_index),
    sp_req_(req),
    sp_res_(new Respond)
{
    sp_res_->status_code = StatusCode::k404_NotFound;
    sp_res_->http_ver = HttpVer::k1_1;
}

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
    return true;
}

}
}
}
