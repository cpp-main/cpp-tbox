#include "request.h"
#include <tbox/base/defines.h>
#include <sstream>

namespace tbox {
namespace http {

bool Request::isValid() const
{
    return method_ != Method::kUnset && http_ver_ != HttpVer::kUnset && !url_.empty();
}

std::string Request::toString() const
{
    std::ostringstream oss;
    oss << method_ << " " << LocalToHttp(url_) << " " << http_ver_ << CRLF;
    for (auto &head : heads_)
        oss << head.first << ": " << head.second << CRLF;
    oss << "Content-length: " << body_.length() + 1 << CRLF;
    oss << CRLF;
    oss << body_;

    return oss.str();
}

}
}
