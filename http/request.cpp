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
    oss << method_ << " " << url_ << " " << http_ver_ << CRLF;
    for (auto &head : headers)
        oss << head.first << ": " << head.second << CRLF;
    oss << "Content-Length: " << body_.length() << CRLF;
    oss << CRLF;
    oss << body_;

    return oss.str();
}

}
}
