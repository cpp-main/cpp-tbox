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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "form_data_middleware.h"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>

#include <tbox/base/assert.h>
#include <tbox/base/log.h>
#include <tbox/util/string.h>

#include "../context.h"
#include "route_key.h"

namespace tbox {
namespace http {
namespace server {

//! 匿名命名空间，这些函数只在本文件内可见
namespace {

/**
 * 解析Content-Type头部中的boundary参数
 */
bool ParseBoundary(const std::string& content_type, std::string &boundary) {
    //! 查找boundary参数
    size_t pos = content_type.find("boundary=");
    if (pos == std::string::npos)
        return false;

    pos += 9;  //! "boundary="的长度

    //! 如果boundary参数值被引号包围，则去掉引号
    if (content_type[pos] == '"') {
        size_t end_pos = content_type.find('"', pos + 1);
        if (end_pos == std::string::npos)
            return false;
        boundary = content_type.substr(pos + 1, end_pos - pos - 1);

    //! 找到参数的结束位置（分号或字符串结束）
    } else {
        size_t end_pos = content_type.find(';', pos);
        if (end_pos == std::string::npos)
            end_pos = content_type.length();
        boundary = content_type.substr(pos, end_pos - pos);
    }
    return true;
}

/**
 * 解析Content-Disposition头部内容
 */
bool ParseContentDisposition(const std::string& header_value, FormItem& item) {
    //! 确保这是一个表单数据字段
    if (header_value.find("form-data") == std::string::npos) {
        LogNotice("Not a form-data Content-Disposition");
        return false;
    }

    //! 解析参数
    size_t pos = 0;
    while ((pos = header_value.find(';', pos)) != std::string::npos) {
        pos++;  //! 跳过分号

        //! 跳过空白字符
        while (pos < header_value.length() && std::isspace(header_value[pos]))
            pos++;

        //! 查找参数名称和值
        size_t eq_pos = header_value.find('=', pos);
        if (eq_pos == std::string::npos)
            continue;

        std::string param_name = header_value.substr(pos, eq_pos - pos);

        //! 处理引号包围的值
        size_t value_start = eq_pos + 1;
        if (header_value[value_start] == '"') {
            value_start++;
            size_t value_end = header_value.find('"', value_start);
            if (value_end == std::string::npos)
                break;

            std::string param_value = header_value.substr(value_start, value_end - value_start);

            if (param_name == "name") {
                item.name = param_value;
                item.type = FormItem::Type::kField;

            } else if (param_name == "filename") {
                item.filename = param_value;
                item.type = FormItem::Type::kFile;
            }

            pos = value_end + 1;
        }
    }

    return true;
}

/**
 * 解析表单项头部
 */
bool ParseFormItemHeaders(const std::string& headers_str, FormItem& item) {
    std::istringstream headers_stream(headers_str);
    std::string line;

    //! 逐行解析头部
    while (std::getline(headers_stream, line)) {
        //! 移除行尾的\r
        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        //! 跳过空行
        if (line.empty())
            continue;

        //! 解析头部字段
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            LogNotice("Invalid header format: %s", line.c_str());
            continue;
        }

        std::string header_name = util::string::Strip(line.substr(0, colon_pos));
        std::string header_value = util::string::Strip(line.substr(colon_pos + 1));

        //! 存储头部
        item.headers[header_name] = header_value;

        //! 处理Content-Disposition头部
        if (util::string::ToLower(header_name) == "content-disposition")
            ParseContentDisposition(header_value, item);

        //! 处理Content-Type头部
        if (util::string::ToLower(header_name) == "content-type")
            item.content_type = header_value;
    }

    return !item.name.empty();
}

/**
 * 解析单个表单部分
 */
bool ParseFormPart(const std::string& part, FormItem& item) {
    //! 分离头部和正文
    size_t headers_end = part.find("\r\n\r\n");
    if (headers_end == std::string::npos) {
        LogNotice("Invalid form part format");
        return false;
    }

    std::string headers_str = part.substr(0, headers_end);
    std::string body = part.substr(headers_end + 4); //! +4 跳过两个CRLF

    //! 解析头部
    if (!ParseFormItemHeaders(headers_str, item))
        return false;

    //! 设置内容
    item.value = body;

    return true;
}

bool ParseAsMultipartFormData(const Request& req, const std::string &content_type, FormData& form_data) {
    //! 解析boundary参数
    std::string boundary;
    if (!ParseBoundary(content_type, boundary)) {
        LogNotice("Failed to parse boundary from Content-Type");
        return false;
    }

    //! 带两个连字符的完整边界
    const std::string delimiter = "--" + boundary;

    //! 分割请求体以获取各个部分
    size_t pos = 0;

    //! 跳过第一个边界
    pos = req.body.find(delimiter, pos);
    if (pos == std::string::npos) {
        LogNotice("Initial boundary not found");
        return false;
    }

    pos += delimiter.length();
    //! 跳过CRLF
    if (req.body.substr(pos, 2) == "\r\n")
        pos += 2;

    //! 查找每个部分
    while (true) {
        //! 查找下一个边界
        size_t next_pos = req.body.find(delimiter, pos);

        //! 没有找到更多边界
        if (next_pos == std::string::npos)
            break;

        //! 提取该部分的内容（不包括前导CRLF）
        const std::string part = req.body.substr(pos, next_pos - pos - 2);  //! 减去末尾的CRLF

        //! 解析表单项
        FormItem item;
        if (ParseFormPart(part, item))
            form_data.addItem(item);

        //! 移动到下一个部分
        pos = next_pos + delimiter.length();

        //! 检查是否是最后一个边界
        if (req.body.substr(pos, 2) == "--")
            break;  //! 这是最后一个边界

        //! 跳过CRLF
        if (req.body.substr(pos, 2) == "\r\n")
            pos += 2;
    }

    return true;
}

bool ParseAsFormUrlEncoded(const Request& req, FormData& form_data) {
    //! 分割请求体以获取各个字段
    const std::string &body = req.body;
    size_t pos = 0;

    while (pos < body.length()) {
        //! 查找下一个字段分隔符（&）
        size_t next_pos = body.find('&', pos);
        if (next_pos == std::string::npos)
            next_pos = body.length();

        //! 提取键值对
        std::string pair = body.substr(pos, next_pos - pos);
        size_t eq_pos = pair.find('=');

        if (eq_pos != std::string::npos) {
            std::string name = UrlDecode(pair.substr(0, eq_pos));
            std::string value = UrlDecode(pair.substr(eq_pos + 1));

            FormItem item;
            item.type = FormItem::Type::kField;
            item.name = name;
            item.value = value;
            form_data.addItem(item);
        }

        //! 移动到下一个字段
        pos = next_pos + 1;
    }

    return true;
}


} //! 匿名命名空间结束

struct FormDataMiddleware::Data {
    std::map<RouteKey, FormHandler> handlers;
};

FormDataMiddleware::FormDataMiddleware() : d_(new Data) { }

FormDataMiddleware::~FormDataMiddleware() { delete d_; }

void FormDataMiddleware::post(const std::string& path, FormHandler&& handler) {
    registerHandler(Method::kPost, path, std::move(handler));
}

void FormDataMiddleware::get(const std::string& path, FormHandler&& handler) {
    registerHandler(Method::kGet, path, std::move(handler));
}

void FormDataMiddleware::registerHandler(Method method, const std::string& path, FormHandler&& handler) {
    RouteKey key{method, path};
    d_->handlers[key] = std::move(handler);
}

void FormDataMiddleware::handle(ContextSptr sp_ctx, const NextFunc& next) {
    Request& req = sp_ctx->req();

    //! 检查路由是否匹配
    RouteKey key{req.method, req.url.path};
    auto it = d_->handlers.find(key);
    if (it == d_->handlers.end()) {
        //! 如果没有匹配，继续下一个中间件
        next();
        return;
    }

    //! 解析表单数据
    FormData form_data;
    bool parsed = false;

    //! 获取Content-Type
    auto content_type_it = req.headers.find("Content-Type");
    if (content_type_it != req.headers.end()) {
        const std::string& content_type = content_type_it->second;
        //! 处理multipart/form-data
        if (content_type.find("multipart/form-data") != std::string::npos) {
            parsed = ParseAsMultipartFormData(req, content_type, form_data);

        //! 处理application/x-www-form-urlencoded
        } else if (content_type.find("application/x-www-form-urlencoded") != std::string::npos) {
            parsed = ParseAsFormUrlEncoded(req, form_data);
        }
    }

    //! 对于GET请求，解析查询参数
    if (req.method == Method::kGet && !req.url.query.empty()) {
        for (const auto& param : req.url.query) {
            FormItem item;
            item.type = FormItem::Type::kField;
            item.name = param.first;
            item.value = param.second;
            form_data.addItem(item);
        }
        parsed = true;
    }

    //! 如果解析成功，则调用处理函数
    if (parsed) {
        it->second(sp_ctx, form_data, next);
    } else {
        //! 如果解析失败，继续下一个中间件
        next();
    }
}

}
}
}
