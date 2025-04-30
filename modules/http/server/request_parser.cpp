/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "request_parser.h"
#include <limits>
#include <tbox/base/defines.h>
#include <tbox/base/log.h>
#include <tbox/util/string.h>

namespace tbox {
namespace http {
namespace server {

RequestParser::~RequestParser()
{
    CHECK_DELETE_RESET_OBJ(sp_request_);
}

size_t RequestParser::parse(const void *data_ptr, size_t data_size)
{
    std::string str(static_cast<const char*>(data_ptr), data_size);
    size_t pos = 0;

    if (state_ == State::kInit) {
        content_length_ = std::numeric_limits<size_t>::max();
        if (sp_request_ == nullptr)
            sp_request_ = new Request;

        //! 获取 method
        auto method_str_end = str.find_first_of(' ', pos);
        auto method_str = str.substr(pos, method_str_end);
        auto method = StringToMethod(method_str);
        if (method == Method::kUnset) {
            LogNotice("method is invalid, method_str:%s", method_str.c_str());
            state_ = State::kFail;
            return pos;
        }

        /* 解析："GET /index.html HTTP/1.1\r\n" */
        auto end_pos = str.find(CRLF, method_str_end);
        if (end_pos == std::string::npos)   //! 如果没有找到首行 \r\n，则放弃
            return 0;

        sp_request_->method = method;

        //! 获取 url
        auto url_str_begin = str.find_first_not_of(' ', method_str_end);
        if (url_str_begin == std::string::npos || url_str_begin >= end_pos) {
            LogNotice("parse url fail, url not exist.");
            state_ = State::kFail;
            return pos;
        }

        auto url_str_end = str.find_first_of(' ', url_str_begin);
        auto url_str = str.substr(url_str_begin, url_str_end - url_str_begin);
        if (!StringToUrlPath(url_str, sp_request_->url)) {
            LogNotice("parse url fail, url_str:%s", url_str.c_str());
            state_ = State::kFail;
            return pos;
        }

        //! 获取版本
        auto ver_str_begin = str.find_first_not_of(' ', url_str_end);
        if (ver_str_begin == std::string::npos || ver_str_begin >= end_pos) {
            LogNotice("ver not exist");
            state_ = State::kFail;
            return pos;
        }

        auto ver_str_end = end_pos;
        auto ver_str = str.substr(ver_str_begin, ver_str_end - ver_str_begin);
        if (ver_str.compare(0, 5, "HTTP/") != 0) {
            LogNotice("ver is invalid, ver_str:%s", ver_str.c_str());
            state_ = State::kFail;
            return pos;
        }

        auto ver = StringToHttpVer(ver_str);
        if (ver == HttpVer::kUnset) {
            LogNotice("ver is invalid, ver_str:%s", ver_str.c_str());
            state_ = State::kFail;
            return pos;
        }

        sp_request_->http_ver = ver;

        pos = end_pos + 2;
        state_ = State::kFinishedStartLine;
    }

    if (state_ == State::kFinishedStartLine) {
        /**
         * 解析：
         *  Content-Length: 12\r\n
         *  Content-Type: plan/text\r\n
         *  s-token: \r\n
         * 直接遇到空白行
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
            if (colon_pos == std::string::npos || colon_pos >= end_pos) {
                LogNotice("can't find ':' in header line");
                state_ = State::kFail;
                return pos;
            }

            auto head_key = util::string::Strip(str.substr(pos, colon_pos - pos));
            auto head_value_start_pos = str.find_first_not_of(' ', colon_pos + 1);  //! 要略掉空白

            if (head_value_start_pos < end_pos) {
                auto head_value_end_pos   = end_pos;
                auto head_value = util::string::Strip(str.substr(head_value_start_pos, head_value_end_pos - head_value_start_pos));
                sp_request_->headers[head_key] = head_value;

                if (head_key == "Content-Length")
                    content_length_ = std::stoi(head_value);

            } else {
                sp_request_->headers[head_key] = "";
            }

            pos = end_pos + 2;
        }
    }
    
    if (state_ == State::kFinishedHeads) {
        if (content_length_ != std::numeric_limits<size_t>::max()) { //! 如果有指定 Content-Lenght
            if ((data_size - pos) >= content_length_) {
                sp_request_->body = str.substr(pos, content_length_);
                pos += content_length_;
                state_ = State::kFinishedAll;
            }
        } else {
            sp_request_->body = str.substr(pos);
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
}
