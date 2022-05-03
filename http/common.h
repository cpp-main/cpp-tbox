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
    kMax
};

//! 方法
enum class Method {
    kUnset,
    kGet,
    kHead,
    kPut,
    kPost, 
    kDelete,
    kMax
};

//! 返回状态码
enum class StatusCode {
    kUnset,

    //! 正常
    k200_OK,
    k201_Created,
    k202_Accepted,
    k203_NonAuthoritativeInformation,
    k204_NoContent,
    k205_ResetContent,
    k206_PartialContent,

    //! 重定向
    k300_MultipleChoices,
    k301_MovedPermanently,
    k302_Found,
    k303_SeeOther,
    k304_NotModified,
    k305_UseProxy,
    k307_TemporaryRedirect,

    //! 客户端出错
    k400_BadRequest,
    k401_Unauthorized,
    k402_PaymentRequired,
    k403_Forbidden,
    k404_NotFound,
    k405_MethodNotAllowed,
    k406_NotAcceptable,
    k407_ProxyAuthenticationRequired,
    k408_RequestTimeout,
    k409_Conflict,
    k410_Gone,
    k411_LengthRequired,
    k412_PreconditionFailed,
    k413_RequestEntityTooLarge,
    k414_RequestURITooLong,
    k415_UnsupportedMediaType,
    k416_RequestedRangeNotSatisfiable,
    k417_ExpectationFailed,

    //! 服务端出错
    k500_InternalServerError,
    k501_NotImplemented,
    k502_BadGateway,
    k503_ServiceUnavailable,
    k504_GatewayTimeout,
    k505_HTTPVersionNotSupported,

    kMax
};

class Context;
using ContextSptr = std::shared_ptr<Context>;

using NextFunc = std::function<void()>;
using RequestCallback = std::function<void(ContextSptr, const NextFunc &)>;

using ConnToken = cabinet::Token;

using Headers = std::map<std::string, std::string>;

//! 字串转义处理

/**
 * \brief   将本地字串转换成Http中的字串，即将特殊字符转换成 %xx 形式
 *          如："hello world" --> "hello%20world"
 */
std::string LocalToHttp(const std::string &local_str);

/**
 * \brief   将Http中的字串，转换成本地。
 *          即将字串中%xx转换成真实的字串，如："hello%20world" -> "hello world"
 * \note    如果遇到%x，不足两个字符的情况则会抛out_of_range异常
 */
std::string HttpToLocal(const std::string &http_str);   //! 注意：有可能会抛异常 out_of_range

}
}

std::ostream& operator << (std::ostream &oss, tbox::http::HttpVer ver);
std::ostream& operator << (std::ostream &oss, tbox::http::Method method);
std::ostream& operator << (std::ostream &oss, tbox::http::StatusCode code);

#endif //TBOX_HTTP_COMMON_H_20220501
