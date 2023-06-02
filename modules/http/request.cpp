#include "request.h"
#include <tbox/base/defines.h>
#include <sstream>

namespace tbox {
namespace http {

bool Request::isValid() const
{
    return method != Method::kUnset && http_ver != HttpVer::kUnset && !url.path.empty();
}

std::string Request::toString() const
{
    std::ostringstream oss;
    oss << MethodToString(method) << " " << UrlPathToString(url) << " " << HttpVerToString(http_ver) << CRLF;
    for (auto &head : headers)
        oss << head.first << ": " << head.second << CRLF;
    oss << "Content-Length: " << body.length() << CRLF;
    oss << CRLF;
    oss << body;

    return oss.str();
}

}
}
