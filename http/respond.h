#ifndef TBOX_HTTP_RESPOND_H_20220501
#define TBOX_HTTP_RESPOND_H_20220501

#include "common.h"

namespace tbox {
namespace http {

//! 回复
struct Respond {
    HttpVer http_ver = HttpVer::kUnset;
    StatusCode status_code = StatusCode::kUnset;
    Headers headers;
    std::string body;

    bool isValid() const;
    std::string toString() const;
};

}
}

#endif //TBOX_HTTP_RESPOND_H_20220501
