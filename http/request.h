#ifndef TBOX_HTTP_REQUEST_H_20220501
#define TBOX_HTTP_REQUEST_H_20220501

#include "common.h"
#include "url.h"

namespace tbox {
namespace http {

//! 请求
struct Request {
    Method  method = Method::kUnset;
    HttpVer http_ver = HttpVer::kUnset;
    Url::Path url;
    Headers headers;
    std::string body;

    bool isValid() const;
    std::string toString() const;
};

}
}

#endif //TBOX_HTTP_REQUEST_H_20220501
