#ifndef TBOX_HTTP_REQUEST_H_20220501
#define TBOX_HTTP_REQUEST_H_20220501

#include "common.h"
#include <map>

namespace tbox {
namespace http {

//! 请求
class Request {
  public:
    bool isValid() const;
    std::string toString() const;

  public:
    void set_method(Method method) { method_ = method; }
    void set_http_ver(HttpVer http_ver) { http_ver_ = http_ver; }
    void set_url(const std::string &url) { url_ = url; }
    void set_body(const std::string &body) { body_ = body; }

    Method method() const { return method_; }
    HttpVer http_ver() const { return http_ver_; }
    std::string url() const { return url_; }
    std::string body() const { return body_; }

    Headers headers;

  private:
    Method  method_ = Method::kUnset;
    HttpVer http_ver_ = HttpVer::kUnset;
    std::string url_;
    std::string body_;
};

}
}

#endif //TBOX_HTTP_REQUEST_H_20220501
