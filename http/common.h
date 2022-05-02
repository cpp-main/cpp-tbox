#ifndef TBOX_HTTP_COMMON_H_20220501
#define TBOX_HTTP_COMMON_H_20220501

#include <memory>
#include <functional>
#include <tbox/base/cabinet_token.h>

namespace tbox {
namespace http {

enum class Ver {
    kHttp_1_0,  //!< http 1.0
    kHttp_1_1,  //!< http 1.1
    kHttp_1_2,  //!< http 1.2
};

//! 方法
enum class Method {
    kGet,
    kHead,
    kPut,
    kPost, 
    kDelete
};

//! 返回状态码
enum class StatusCode {
    k200_OK,
    k401_Unauthorized,
    k404_NotFound
};

class Request;
class Respond;

using RequestSptr = std::shared_ptr<Request>;
using RespondSptr = std::shared_ptr<Respond>;

using NextFunc = std::function<void()>;
using RequestCallback = std::function<void(RequestSptr, RespondSptr, const NextFunc &)>;

using ConnToken = cabinet::Token;

}
}

#endif //TBOX_HTTP_COMMON_H_20220501
