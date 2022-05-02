#ifndef TBOX_HTTP_CONTEXT_H_20220502
#define TBOX_HTTP_CONTEXT_H_20220502

#include "common.h"
#include "server_imp.h"
#include "context.h"
#include "request.h"
#include "respond.h"

namespace tbox {
namespace http {

/**
 * Http请求上下文
 */
class Context {
  public:
    Context(Server::Impl *wp_server, const ConnToken &ct, int req_index, Request *req);
    ~Context();

    NONCOPYABLE(Context);

  public:
    Request& req() const { return *sp_req_; }
    Respond& res() const { return *sp_res_; }

    bool done();

  private:
    Server::Impl *wp_server_;
    ConnToken     conn_token_;
    int           req_index_;

    Request    *sp_req_;
    Respond    *sp_res_;
    bool        is_done_ = false;
};

}
}

#endif //TBOX_HTTP_CONTEXT_H_20220502
