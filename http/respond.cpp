#include "respond.h"
#include <sstream>

namespace tbox {
namespace http {

bool Respond::isValid() const
{
    return status_code_ != StatusCode::kUnset && http_ver_ != HttpVer::kUnset;
}

std::string Respond::toString() const
{
    std::ostringstream oss;
    oss << http_ver_ << " " << status_code_ << CRLF;
    for (auto &head : heads_)
        oss << head.first << ": " << head.second << CRLF;
    oss << "Content-length: " << body_.length() + 1 << CRLF;
    oss << CRLF;
    oss << body_;

    return oss.str();
}

}
}

