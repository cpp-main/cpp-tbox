#include "request_parser.h"
#include <limits>
#include <tbox/base/defines.h>

namespace tbox {
namespace http {

namespace {
const int kStartLineMinLen = 14;
}

RequestParser::~RequestParser()
{
    CHECK_DELETE_RESET_OBJ(sp_request_);
}

#if 0
GET /index.html HTTP/1.1\r\n
Content-Length: 12\r\n
Content-Type: plan/text\r\n
\r\n
hello world!
#endif
size_t RequestParser::parse(const void *data_ptr, size_t data_size)
{
    std::string str(static_cast<const char*>(data_ptr), data_size);
    size_t pos = 0;

    if (state_ == State::kInit) {
        content_length_ = std::numeric_limits<size_t>::max();
        if (sp_request_ == nullptr)
            sp_request_ = new Request;

        /* 解析："GET /index.html HTTP/1.1\r\n" */
        auto end_pos = str.find(CRLF, kStartLineMinLen);
        if (end_pos == std::string::npos)   //! 如果没有找到首行 \r\n，则放弃
            return 0;

        auto method_str_end = str.find_first_of(' ', pos);
        auto method_str = str.substr(pos, method_str_end);
        sp_request_->set_method(StringToMethod(method_str));

        auto url_str_begin = str.find_first_not_of(' ', method_str_end);
        auto url_str_end = str.find_first_of(' ', url_str_begin);
        auto url_str = str.substr(url_str_begin, url_str_end - url_str_begin);
        sp_request_->set_url(url_str);

        auto ver_str_begin = str.find_first_not_of(' ', url_str_end);
        auto ver_str_end = end_pos;
        auto ver_str = str.substr(ver_str_begin, ver_str_end - ver_str_begin);
        sp_request_->set_http_ver(StringToHttpVer(ver_str));

        pos = end_pos + 2;
        state_ = State::kFinishedStartLine;
    }

    if (state_ == State::kFinishedStartLine) {
        /**
         * 解析：
         *  Content-Length: 12\r\n
         *  Content-Type: plan/text\r\n
         */
        for (;;) {
            auto end_pos = str.find(CRLF, pos);

            if (end_pos == pos) {   //! 找到了空白行
                state_ = State::kFinishedHeads;
                pos += 2;
                break;
            } else if (end_pos == std::string::npos) {  //! 当前的Head不完整
                break;
            }

            auto colon_pos = str.find_first_of(':', pos);
            auto head_key = str.substr(pos, colon_pos - pos);
            auto head_value_start_pos = str.find_first_not_of(' ', colon_pos + 1);  //! 要略掉空白
            auto head_value_end_pos   = end_pos;
            auto head_value = str.substr(head_value_start_pos, head_value_end_pos - head_value_start_pos);
            sp_request_->headers[head_key] = head_value;

            if (head_key == "Content-Length")
                content_length_ = std::stoi(head_value);

            pos = end_pos + 2;
        }
    }
    
    if (state_ == State::kFinishedHeads) {
        if (content_length_ != std::numeric_limits<size_t>::max()) { //! 如果有指定 Content-Lenght
            if ((data_size - pos) >= content_length_) {
                sp_request_->set_body(str.substr(pos, content_length_));
                pos += content_length_;
                state_ = State::kFinishedAll;
            }
        } else {
            sp_request_->set_body(str.substr(pos));
            pos = data_size;
            state_ = State::kFinishedAll;
        }
    }

    return pos;
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
