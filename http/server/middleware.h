#ifndef TBOX_HTTP_MIDDLEWARE_H_20220501
#define TBOX_HTTP_MIDDLEWARE_H_20220501

#include "types.h"

namespace tbox {
namespace http {
namespace server {

//! 中间件
class Middleware {
  public:
    ~Middleware() { }

  public:
    virtual void handle(ContextSptr sp_ctx, const NextFunc &next) = 0;
};

}
}
}

#endif //TBOX_HTTP_MIDDLEWARE_H_20220501
