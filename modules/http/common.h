/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_HTTP_COMMON_H_20220501
#define TBOX_HTTP_COMMON_H_20220501

#include <string>
#include <map>

#define CRLF "\r\n"

namespace tbox {
namespace http {

using Headers = std::map<std::string, std::string>;

enum class HttpVer {
    kUnset,
    k1_0,  //!< http 1.0
    k1_1,  //!< http 1.1
    k2_0,  //!< http 2.0
    kMax
};

std::string HttpVerToString(HttpVer ver);
HttpVer     StringToHttpVer(const std::string &str);

//! 方法
enum class Method {
    kUnset,
    kGet,
    kHead,
    kPut,
    kPost, 
    kTrace,
    kOptions,
    kDelete,
    kMax
};

std::string MethodToString(Method ver);
Method      StringToMethod(const std::string &str);

//! 返回状态码
enum class StatusCode {
    kUnset,

    //! 正常
    k200_OK = 200,
    k201_Created = 201,
    k202_Accepted = 202,
    k203_NonAuthoritativeInformation = 203,
    k204_NoContent = 204,
    k205_ResetContent = 205,
    k206_PartialContent = 206,

    //! 重定向
    k300_MultipleChoices = 300,
    k301_MovedPermanently = 301,
    k302_Found = 302,
    k303_SeeOther = 303,
    k304_NotModified = 304,
    k305_UseProxy = 305,
    k307_TemporaryRedirect = 307,

    //! 客户端出错
    k400_BadRequest = 400,
    k401_Unauthorized = 401,
    k402_PaymentRequired = 402,
    k403_Forbidden = 403,
    k404_NotFound = 404,
    k405_MethodNotAllowed = 405,
    k406_NotAcceptable = 406,
    k407_ProxyAuthenticationRequired = 407,
    k408_RequestTimeout = 408,
    k409_Conflict = 409,
    k410_Gone = 410,
    k411_LengthRequired = 411,
    k412_PreconditionFailed = 412,
    k413_RequestEntityTooLarge = 413,
    k414_RequestURITooLong = 414,
    k415_UnsupportedMediaType = 415,
    k416_RequestedRangeNotSatisfiable = 416,
    k417_ExpectationFailed = 417,

    //! 服务端出错
    k500_InternalServerError = 500,
    k501_NotImplemented = 501,
    k502_BadGateway = 502,
    k503_ServiceUnavailable = 503,
    k504_GatewayTimeout = 504,
    k505_HTTPVersionNotSupported = 505,

    kMax
};

std::string StatusCodeToString(StatusCode ver);
StatusCode  StringToStatusCode(const std::string &str);

}
}

#endif //TBOX_HTTP_COMMON_H_20220501
