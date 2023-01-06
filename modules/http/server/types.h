#ifndef TBOX_HTTP_SERVER_TYPES_H_20220503
#define TBOX_HTTP_SERVER_TYPES_H_20220503

#include <memory>
#include <functional>

namespace tbox {
namespace http {
namespace server {

class Context;
using ContextSptr = std::shared_ptr<Context>;
using NextFunc = std::function<void()>;
using RequestCallback = std::function<void(ContextSptr, const NextFunc &)>;

}
}
}

#endif //TBOX_HTTP_SERVER_TYPES_H_20220503
