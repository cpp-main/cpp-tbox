#ifndef TBOX_HTTP_SERVER_ROUTER_H_20220508
#define TBOX_HTTP_SERVER_ROUTER_H_20220508

#include "middleware.h"
#include "context.h"

namespace tbox {
namespace http {
namespace server {

class Router : public Middleware {
  public:
    Router();
    ~Router();

  public:
    Router& get (const std::string &path, const RequestCallback &cb);
    Router& post(const std::string &path, const RequestCallback &cb);
    Router& put (const std::string &path, const RequestCallback &cb);
    Router& del (const std::string &path, const RequestCallback &cb);

  public:
    virtual void handle(ContextSptr sp_ctx, const NextFunc &next) override;

  private:
    struct Data;
    Data *d_;
};

}
}
}

#endif //TBOX_HTTP_SERVER_ROUTER_H_20220508
