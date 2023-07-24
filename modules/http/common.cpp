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
#include "common.h"
#include <algorithm>
#include <tbox/base/defines.h>

namespace tbox {
namespace http {

namespace {
using HttpVerPair = std::pair<HttpVer, std::string>;
HttpVerPair _http_ver_map[] = {
    { HttpVer::k1_0, "HTTP/1.0" },
    { HttpVer::k1_1, "HTTP/1.1" },
    { HttpVer::k2_0, "HTTP/2.0" },
};
}

std::string HttpVerToString(HttpVer ver)
{
    auto table_begin = _http_ver_map;
    auto table_end   = _http_ver_map + NUMBER_OF_ARRAY(_http_ver_map);
    auto iter = std::find_if(table_begin, table_end,
        [=] (const HttpVerPair &item) {
           return item.first == ver;
        });
    if (iter != table_end)
        return iter->second;
    else
        return "";
}

HttpVer StringToHttpVer(const std::string &str)
{
    auto table_begin = _http_ver_map;
    auto table_end   = _http_ver_map + NUMBER_OF_ARRAY(_http_ver_map);
    auto iter = std::find_if(table_begin, table_end,
        [=] (const HttpVerPair &item) {
           return item.second == str;
        });

    if (iter != table_end)
        return iter->first;
    else
        return HttpVer::kUnset;
}

namespace {
using MethodPair = std::pair<Method, std::string>;
MethodPair _method_map[] = {
    { Method::kGet,     "GET" },
    { Method::kHead,    "HEAD" },
    { Method::kPut,     "PUT" },
    { Method::kPost,    "POST" },
    { Method::kTrace,   "TRACE" },
    { Method::kOptions, "OPTIONS" },
    { Method::kDelete,  "DELETE" },
};
}

std::string MethodToString(Method ver)
{
    auto table_begin = _method_map;
    auto table_end   = _method_map + NUMBER_OF_ARRAY(_method_map);
    auto iter = std::find_if(table_begin, table_end,
        [=] (const MethodPair &item) {
           return item.first == ver;
        });
    if (iter != table_end)
        return iter->second;
    else
        return "";
}

Method StringToMethod(const std::string &str)
{
    auto table_begin = _method_map;
    auto table_end   = _method_map + NUMBER_OF_ARRAY(_method_map);
    auto iter = std::find_if(table_begin, table_end,
        [=] (const MethodPair &item) {
           return item.second == str;
        });

    if (iter != table_end)
        return iter->first;
    else
        return Method::kUnset;
}

namespace {
using StatusCodePair = std::pair<StatusCode, std::string>;
StatusCodePair _status_code_map[] = {
    { StatusCode::k200_OK, "200 OK"},
    { StatusCode::k201_Created, "201 Created"},
    { StatusCode::k202_Accepted, "202 Accepted"},
    { StatusCode::k203_NonAuthoritativeInformation, "203 Non-Authoritative Information"},
    { StatusCode::k204_NoContent, "204 No Content"},
    { StatusCode::k205_ResetContent, "205 Reset Content"},
    { StatusCode::k206_PartialContent, "206 Partial Content"},
    { StatusCode::k300_MultipleChoices, "300 Multiple Choices"},
    { StatusCode::k301_MovedPermanently, "301 Moved Permanently"},
    { StatusCode::k302_Found, "302 Found"},
    { StatusCode::k303_SeeOther, "303 See Other"},
    { StatusCode::k304_NotModified, "304 Not Modified"},
    { StatusCode::k305_UseProxy, "305 Use Proxy"},
    { StatusCode::k307_TemporaryRedirect, "307 Temporary Redirect"},
    { StatusCode::k400_BadRequest, "400 Bad Request"},
    { StatusCode::k401_Unauthorized, "401 Unauthorized"},
    { StatusCode::k402_PaymentRequired, "402 Payment Required"},
    { StatusCode::k403_Forbidden, "403 Forbidden"},
    { StatusCode::k404_NotFound, "404 Not Found"},
    { StatusCode::k405_MethodNotAllowed, "405 Method Not Allowed"},
    { StatusCode::k406_NotAcceptable, "406 Not Acceptable"},
    { StatusCode::k407_ProxyAuthenticationRequired, "407 Proxy Authentication Required"},
    { StatusCode::k408_RequestTimeout, "408 Request Timeout"},
    { StatusCode::k409_Conflict, "409 Conflict"},
    { StatusCode::k410_Gone, "410 Gone"},
    { StatusCode::k411_LengthRequired, "411 Length Required"},
    { StatusCode::k412_PreconditionFailed, "412 Precondition Failed"},
    { StatusCode::k413_RequestEntityTooLarge, "413 Request Entity Too Large"},
    { StatusCode::k414_RequestURITooLong, "414 Request URI Too Long"},
    { StatusCode::k415_UnsupportedMediaType, "415 Unsupported Media Type"},
    { StatusCode::k416_RequestedRangeNotSatisfiable, "416 Requested Range Not Satisfiable"},
    { StatusCode::k417_ExpectationFailed, "417 Expectation Failed"},
    { StatusCode::k500_InternalServerError, "500 Internal Server Error"},
    { StatusCode::k501_NotImplemented, "501 Not Implemented"},
    { StatusCode::k502_BadGateway, "502 Bad Gateway"},
    { StatusCode::k503_ServiceUnavailable, "503 Service Unavailable"},
    { StatusCode::k504_GatewayTimeout, "504 Gateway Timeout"},
    { StatusCode::k505_HTTPVersionNotSupported, "505 HTTP Version Not Supported"},
};
}

std::string StatusCodeToString(StatusCode code)
{
    auto table_begin = _status_code_map;
    auto table_end   = _status_code_map + NUMBER_OF_ARRAY(_status_code_map);
    auto iter = std::find_if(table_begin, table_end,
        [=] (const StatusCodePair &item) {
           return item.first == code;
        });
    if (iter != table_end)
        return iter->second;
    else
        return "";
}

StatusCode StringToStatusCode(const std::string &str)
{
    auto table_begin = _status_code_map;
    auto table_end   = _status_code_map + NUMBER_OF_ARRAY(_status_code_map);
    auto iter = std::find_if(table_begin, table_end,
        [=] (const StatusCodePair &item) {
           return item.second == str;
        });

    if (iter != table_end)
        return iter->first;
    else
        return StatusCode::kUnset;
}

}
}
