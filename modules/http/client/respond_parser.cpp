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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "respond_parser.h"
#include <limits>
#include <tbox/base/defines.h>
#include <tbox/base/log.h>
#include <tbox/util/string.h>
#include <tbox/util/string_to.h>

namespace tbox {
namespace http {
namespace client {

RespondParser::~RespondParser()
{
    CHECK_DELETE_RESET_OBJ(sp_respond_);
}

size_t RespondParser::parse(const void *data_ptr, size_t data_size)
{
    std::string str(static_cast<const char*>(data_ptr), data_size);
    size_t pos = 0;

    if (state_ == State::kInit) {
        content_length_ = std::numeric_limits<size_t>::max();
        if (sp_respond_ == nullptr)
            sp_respond_ = new Respond;

        //! 解析首行："HTTP/1.1 200 OK\r\n"
        auto end_pos = str.find(CRLF, pos);
        if (end_pos == std::string::npos)   //! 首行不完整
            return 0;

        //! 提取 HTTP 版本
        auto space1_pos = str.find_first_of(' ', pos);
        if (space1_pos == std::string::npos || space1_pos >= end_pos) {
            LogNotice("respond start line format invalid");
            state_ = State::kFail;
            return pos;
        }

        auto ver_str = str.substr(pos, space1_pos - pos);
        if (ver_str.compare(0, 5, "HTTP/") != 0) {
            LogNotice("respond version invalid, ver_str:%s", ver_str.c_str());
            state_ = State::kFail;
            return pos;
        }

        auto ver = StringToHttpVer(ver_str);
        if (ver == HttpVer::kUnset) {
            LogNotice("respond version invalid, ver_str:%s", ver_str.c_str());
            state_ = State::kFail;
            return pos;
        }

        sp_respond_->http_ver = ver;

        //! 提取状态码
        auto code_begin = str.find_first_not_of(' ', space1_pos);
        if (code_begin == std::string::npos || code_begin >= end_pos) {
            LogNotice("respond status code not exist");
            state_ = State::kFail;
            return pos;
        }

        auto code_end = str.find_first_of(' ', code_begin);
        if (code_end == std::string::npos || code_end > end_pos)
            code_end = end_pos;

        auto code_str = str.substr(code_begin, code_end - code_begin);
        int status_code_num = 0;
        if (!util::StringTo(code_str, status_code_num)) {
            LogNotice("respond status code not number, code_str:%s", code_str.c_str());
            state_ = State::kFail;
            return pos;
        }

        auto status_code = StringToStatusCode(std::to_string(status_code_num));
        if (status_code == StatusCode::kUnset) {
            LogNotice("respond status code invalid, code:%d", status_code_num);
            state_ = State::kFail;
            return pos;
        }

        sp_respond_->status_code = status_code;

        pos = end_pos + 2;
        state_ = State::kFinishedStartLine;
    }

    if (state_ == State::kFinishedStartLine) {
        //! 解析 headers："Key: Value\r\n" 直到空行 "\r\n"
        for (;;) {
            auto end_pos = str.find(CRLF, pos);

            if (end_pos == pos) {   //! 找到了空行，headers 结束
                state_ = State::kFinishedHeads;
                pos += 2;
                break;

            } else if (end_pos == std::string::npos) {  //! 当前的 header 不完整
                break;
            }

            auto colon_pos = str.find_first_of(':', pos);
            if (colon_pos == std::string::npos || colon_pos >= end_pos) {
                LogNotice("can't find ':' in header line");
                state_ = State::kFail;
                return pos;
            }

            auto head_key = util::string::Strip(str.substr(pos, colon_pos - pos));
            auto head_value_start_pos = str.find_first_not_of(' ', colon_pos + 1);

            if (head_value_start_pos < end_pos) {
                auto head_value = util::string::Strip(str.substr(head_value_start_pos, end_pos - head_value_start_pos));
                sp_respond_->headers[head_key] = head_value;

                if (head_key == "Content-Length") {
                    if (!util::StringTo(head_value, content_length_)) {
                        LogNotice("Content-Length should be number");
                        state_ = State::kFail;
                        return pos;
                    }
                }
            } else {
                sp_respond_->headers[head_key] = "";
            }

            pos = end_pos + 2;
        }
    }

    if (state_ == State::kFinishedHeads) {
        if (content_length_ != std::numeric_limits<size_t>::max()) { //! 有 Content-Length
            if ((data_size - pos) >= content_length_) {
                sp_respond_->body = str.substr(pos, content_length_);
                pos += content_length_;
                state_ = State::kFinishedAll;
            }
        } else {
            //! 没有 Content-Length，body 到连接关闭为止（本次全部当作 body）
            if (data_size > pos)
                sp_respond_->body = str.substr(pos);
            pos = data_size;
            state_ = State::kFinishedAll;
        }
    }

    return pos;
}

Respond* RespondParser::getRespond()
{
    Respond *ret = nullptr;
    if (state_ == State::kFinishedAll) {
        std::swap(ret, sp_respond_);
        state_ = State::kInit;
    }
    return ret;
}

void RespondParser::reset()
{
    CHECK_DELETE_RESET_OBJ(sp_respond_);
    state_ = State::kInit;
    content_length_ = std::numeric_limits<size_t>::max();
}

}
}
}
