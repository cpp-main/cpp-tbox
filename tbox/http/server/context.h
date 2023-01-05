#ifndef TBOX_HTTP_CONTEXT_H_20220502
#define TBOX_HTTP_CONTEXT_H_20220502

#include <tbox/base/defines.h>
#include <tbox/base/cabinet_token.h>

#include "../common.h"
#include "../request.h"
#include "../respond.h"

namespace tbox {
namespace http {
namespace server {

class Server;

/**
 * Http请求上下文
 */
class Context {
  public:
    Context(Server *wp_server, const cabinet::Token &ct,
            int req_index, Request *req);
    ~Context();

    NONCOPYABLE(Context);

  public:
    Request& req() const;
    Respond& res() const;   //! 注意: 在 done() 之后就不可以再使用该函数

  private:
    struct Data;
    Data *d_;
};

}
}
}

#endif //TBOX_HTTP_CONTEXT_H_20220502
