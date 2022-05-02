#ifndef TBOX_HTTP_COMMON_H_20220501
#define TBOX_HTTP_COMMON_H_20220501

#include <ostream>
#include <memory>
#include <functional>
#include <map>
#include <tbox/base/cabinet_token.h>

#define CRLF "\r\n";

namespace tbox {
namespace http {

enum class HttpVer {
    kUnset,
    k1_0,  //!< http 1.0
    k1_1,  //!< http 1.1
    k1_2,  //!< http 1.2
};

//! 方法
enum class Method {
    kUnset,
    kGet,
    kHead,
    kPut,
    kPost, 
    kDelete
};

//! 返回状态码
enum class StatusCode {
    kUnset,
    k200_OK,
    k401_Unauthorized,
    k404_NotFound
};

class Context;
using ContextSptr = std::shared_ptr<Context>;

using NextFunc = std::function<void()>;
using RequestCallback = std::function<void(ContextSptr, const NextFunc &)>;

using ConnToken = cabinet::Token;

using Headers = std::map<std::string, std::string>;

}
}

std::ostream& operator << (std::ostream &oss, tbox::http::HttpVer ver);
std::ostream& operator << (std::ostream &oss, tbox::http::Method method);
std::ostream& operator << (std::ostream &oss, tbox::http::StatusCode code);

//! 字串转义处理
std::string LocalToHttp(const std::string &local);
std::string HttpToLocal(const std::string &http);

#endif //TBOX_HTTP_COMMON_H_20220501
