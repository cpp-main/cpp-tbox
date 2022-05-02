#include "request_parser.h"
#include <tbox/base/defines.h>

namespace tbox {
namespace http {

RequestParser::~RequestParser()
{
    CHECK_DELETE_RESET_OBJ(sp_request_);
}

size_t RequestParser::parse(const void *data_ptr, size_t data_size)
{
    const char *str = static_cast<const char*>(data_ptr);
    size_t pos = 0;

    if (state_ == State::kInit) {

    }

    if (state_ == State::kFinishedStartLine) {

    }
    
    if (state_ == State::kFinishedHeads) {

    }

    return 0;
}

Request* RequestParser::getRequest()
{
    Request *ret = nullptr;
    if (state_ == State::kFinishedAll) {
        std::swap(ret, sp_request_);
        state_ = State::kInit;
    }
    return ret;
}

void RequestParser::swap(RequestParser &other)
{
    if (&other != this) {
        std::swap(sp_request_, other.sp_request_);
        std::swap(state_, other.state_);
    }
}

void RequestParser::reset()
{
    RequestParser tmp;
    swap(tmp);
}

}
}
