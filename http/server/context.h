#ifndef TBOX_HTTP_CONTEXT_H_20220502
#define TBOX_HTTP_CONTEXT_H_20220502

#include <tbox/base/defines.h>
#include <tbox/base/cabinet_token.h>

#include "../common.h"
#include "../request.h"
#include "../respond.h"
#include "server.h"

namespace tbox {
namespace http {
namespace server {

/**
 * Http请求上下文
 */
class Context {
  public:
    Context(Server::Impl *wp_server, const cabinet::Token &ct,
            int req_index, Request *req);
    ~Context();

    NONCOPYABLE(Context);

  public:
    Request& req() const { return *sp_req_; }
    Respond& res() const { return *sp_res_; }

    bool done();

  private:
    Server::Impl *wp_server_;
    cabinet::Token conn_token_;
    int req_index_;

    Request    *sp_req_;
    Respond    *sp_res_;
    bool        is_done_ = false;
};

}
}
}

#endif //TBOX_HTTP_CONTEXT_H_20220502
