#include "context.h"
#include "server_imp.h"

namespace tbox {
namespace http {
namespace server {

struct Context::Data {
    Server::Impl *wp_server;
    cabinet::Token conn_token;
    int req_index;

    Request    *sp_req;
    Respond    *sp_res;
    bool        is_done;
};

Context::Context(Server *wp_server, const cabinet::Token &ct, int req_index, Request *req) :
    d_(new Data{ wp_server->impl_, ct, req_index, req, new Respond, false })
{
    d_->sp_res->status_code = StatusCode::k404_NotFound;
    d_->sp_res->http_ver = HttpVer::k1_1;
}

Context::~Context()
{
    if (!d_->is_done)
        done();

    CHECK_DELETE_RESET_OBJ(d_->sp_req);
    CHECK_DELETE_RESET_OBJ(d_->sp_res);
    CHECK_DELETE_RESET_OBJ(d_);
}

Request& Context::req() const
{
    return *(d_->sp_req);
}

Respond& Context::res() const
{
    return *(d_->sp_res);
}

bool Context::done()
{
    if (d_->is_done)
        return false;

    d_->wp_server->commitRespond(d_->conn_token, d_->req_index, d_->sp_res->toString());
    return true;
}

}
}
}
