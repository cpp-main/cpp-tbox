#ifndef TBOX_HTTP_RESPOND_H_20220501
#define TBOX_HTTP_RESPOND_H_20220501

#include "common.h"

namespace tbox {
namespace http {

class Respond {
  public:
    bool isValid() const;
    std::string toString() const;

  public:
    void set_status_code(StatusCode code) { status_code_ = code; }
    void set_http_ver(HttpVer http_ver) { http_ver_ = http_ver; }
    void set_body(const std::string &body) { body_ = body; }

    StatusCode status_code() const { return status_code_; }
    HttpVer http_ver() const { return http_ver_; }
    std::string body() const { return body_; }

    Headers headers;

  private:
    StatusCode status_code_ = StatusCode::kUnset;
    HttpVer http_ver_ = HttpVer::kUnset;
    std::string body_;

};

}
}

#endif //TBOX_HTTP_RESPOND_H_20220501
