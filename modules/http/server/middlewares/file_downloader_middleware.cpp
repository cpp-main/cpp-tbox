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
#include "file_downloader_middleware.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <ctime>

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/util/string.h>
#include <tbox/util/fs.h>
#include <tbox/util/string_to.h>
#include <tbox/eventx/work_thread.h>
#include <tbox/base/defines.h>
#include <tbox/base/recorder.h>

namespace tbox {
namespace http {
namespace server {

namespace {

bool IsPathSafe(const std::string& path)
{
    //! 检查是否有".."路径组件，这可能导致目录遍历
    std::istringstream path_stream(path);
    std::string component;

    while (std::getline(path_stream, component, '/')) {
        if (component == "..")
            return false; //! 不允许上级目录访问
    }

    return true;
}

std::string GenerateETag(time_t mtime, off_t size)
{
    return "\"" + std::to_string(static_cast<long long>(mtime))
        + "-" + std::to_string(static_cast<long long>(size)) + "\"";
}

std::string HttpDateToString(time_t t)
{
    struct tm tm_buf;
    gmtime_r(&t, &tm_buf);
    char buf[64];
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", &tm_buf);
    return buf;
}

time_t StringToHttpDate(const std::string& s)
{
    struct tm t = {};
    if (strptime(s.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &t) == nullptr)
        return -1;
    return timegm(&t);
}

std::string GetHeader(const tbox::http::Headers& headers, const std::string& lower_name)
{
    for (const auto& h : headers) {
        if (tbox::util::string::ToLower(h.first) == lower_name)
            return h.second;
    }
    return "";
}

//! 解析
bool ParseRangeString(const std::string &range_str, size_t file_size, size_t &range_begin, size_t &range_end)
{
    std::string range_val = range_str.substr(6);
    auto dash_pos = range_val.find('-');
    if (dash_pos == std::string::npos)
        return false;

    //!FIXME: 暂不支持 1000-1999, 3000-3555, 多段返回的情况
    auto comma_pos = range_val.find(',', dash_pos);
    if (comma_pos != std::string::npos) {
        return false;
    }

    std::string begin_str = range_val.substr(0, dash_pos);
    std::string end_str   = range_val.substr(dash_pos + 1);

    if (begin_str.empty()) {    //! 出现：-500
        size_t suffix_size = 0;
        if (!util::StringTo(end_str, suffix_size))
            return false;

        range_begin = (suffix_size >= file_size) ? 0 : file_size - suffix_size;
        range_end   = file_size - 1;

    } else {    //! 出现：1000-1999 或 1000-
        if (!util::StringTo(begin_str, range_begin))
            return false;

        if (!end_str.empty()) {
            if (!util::StringTo(end_str, range_end))
                return false;
        } else {
            range_end = file_size - 1;
        }
    }

    return true;
}

}

//! 目录配置项
struct DirectoryConfig {
    std::string url_prefix;   //! URL前缀
    std::string local_path;   //! 本地路径
    std::string default_file; //! 默认文件
};

//! 中间件私有数据结构
struct FileDownloaderMiddleware::Data {
    eventx::ThreadExecutor *worker      = nullptr;
    eventx::WorkThread     *inner_worker = nullptr;
    std::vector<DirectoryConfig> directories;        //! 目录配置列表
    std::map<std::string, std::string> path_mappings;//! 特定路径映射
    std::map<std::string, std::string> mime_types;   //! MIME类型映射
    std::string default_mime_type;                   //! 默认MIME类型
    bool directory_listing_enabled;                  //! 是否允许目录列表
    size_t switch_to_worker_filesize_threshold;

    Data(event::Loop *wp_loop, eventx::ThreadExecutor *wp_thread_executor)
        : worker(wp_thread_executor)
        , default_mime_type("application/octet-stream")
        , directory_listing_enabled(false)
        , switch_to_worker_filesize_threshold(100 << 10)
    {
        if (worker == nullptr) {
            inner_worker = new eventx::WorkThread(wp_loop);
            worker = inner_worker;
        }
        //! 初始化常见MIME类型
        mime_types["html"] = "text/html";
        mime_types["htm"] = "text/html";
        mime_types["css"] = "text/css";
        mime_types["js"] = "application/javascript";
        mime_types["json"] = "application/json";
        mime_types["xml"] = "application/xml";
        mime_types["txt"] = "text/plain";
        mime_types["png"] = "image/png";
        mime_types["jpg"] = "image/jpeg";
        mime_types["jpeg"] = "image/jpeg";
        mime_types["gif"] = "image/gif";
        mime_types["webp"] = "image/webp";
        mime_types["svg"] = "image/svg+xml";
        mime_types["ico"] = "image/x-icon";
        mime_types["pdf"] = "application/pdf";
        mime_types["zip"] = "application/zip";
        mime_types["tar"] = "application/x-tar";
        mime_types["gz"] = "application/gzip";
        mime_types["mp3"] = "audio/mpeg";
        mime_types["mp4"] = "video/mp4";
        mime_types["woff"] = "font/woff";
        mime_types["woff2"] = "font/woff2";
        mime_types["ttf"] = "font/ttf";
        mime_types["otf"] = "font/otf";
    }

    ~Data()
    {
        CHECK_DELETE_RESET_OBJ(inner_worker);
    }
};

FileDownloaderMiddleware::FileDownloaderMiddleware(event::Loop *wp_loop, eventx::ThreadExecutor *wp_thread_executor)
  : d_(new Data(wp_loop, wp_thread_executor))
{ }

FileDownloaderMiddleware::~FileDownloaderMiddleware()
{ delete d_; }

bool FileDownloaderMiddleware::addDirectory(const std::string& url_prefix,
                                            const std::string& local_path,
                                            const std::string& default_file)
{
    //! 验证URL前缀是否以'/'开头
    if (url_prefix.empty() || url_prefix[0] != '/') {
        LogErr("Invalid URL prefix: %s. Must start with '/'", url_prefix.c_str());
        return false;
    }

    //! 验证本地路径是否存在且是目录
    if (!util::fs::IsDirectoryExist(local_path)) {
        LogErr("Invalid local path: %s. Directory does not exist", local_path.c_str());
        return false;
    }

    //! 添加到目录列表
    DirectoryConfig config;
    config.url_prefix = url_prefix;
    config.local_path = local_path;
    config.default_file = default_file;

    //! 确保本地路径以'/'结尾
    if (!config.local_path.empty() && config.local_path.back() != '/')
        config.local_path += '/';

    d_->directories.push_back(config);
    LogInfo("Added directory mapping: %s -> %s", url_prefix.c_str(), local_path.c_str());
    return true;
}

void FileDownloaderMiddleware::setDirectoryListingEnabled(bool enable)
{
    d_->directory_listing_enabled = enable;
}

void FileDownloaderMiddleware::setPathMapping(const std::string& url, const std::string& file)
{
    d_->path_mappings[url] = file;
}

void FileDownloaderMiddleware::setDefaultMimeType(const std::string& mime_type)
{
    d_->default_mime_type = mime_type;
}

void FileDownloaderMiddleware::setMimeType(const std::string& ext, const std::string& mime_type)
{
    d_->mime_types[ext] = mime_type;
}

void FileDownloaderMiddleware::handle(ContextSptr sp_ctx, const NextFunc& next)
{
    const auto& request = sp_ctx->req();

    //! 处理 OPTIONS 预检请求（浏览器跨域访问）
    if (request.method == Method::kOptions) {
        auto& res = sp_ctx->res();
        res.status_code = StatusCode::k200_OK;
        res.headers["Access-Control-Allow-Origin"]          = "*";
        res.headers["Access-Control-Allow-Methods"]         = "GET, HEAD, OPTIONS";
        res.headers["Access-Control-Allow-Headers"]         = "Auth, Range, If-Range, If-None-Match, If-Modified-Since";
        res.headers["Access-Control-Allow-Private-Network"] = "true";
        res.headers["Access-Control-Max-Age"]               = "86400";
        return;
    }

    //! 只处理GET和HEAD请求
    if (request.method != Method::kGet && request.method != Method::kHead) {
        next();
        return;
    }

    const std::string& request_path = request.url.path;

    //! 检查特定路径映射
    auto mapping_it = d_->path_mappings.find(request_path);
    if (mapping_it != d_->path_mappings.end()) {
        if (respondFile(sp_ctx, mapping_it->second))
            return;
    }

    //! 查找匹配的目录配置
    for (const auto& dir : d_->directories) {
        //! 检查URL是否以该目录前缀开头
        if (tbox::util::string::IsStartWith(request_path, dir.url_prefix)) {
            //! 获取相对路径部分
            std::string rel_path = request_path.substr(dir.url_prefix.length());

            //! 如果路径以'/'开头，去掉这个斜杠避免双斜杠
            if (!rel_path.empty() && rel_path[0] == '/')
                rel_path = rel_path.substr(1);

            //! 构造本地文件路径
            std::string file_path = dir.local_path + rel_path;

            //! 检查路径安全性
            if (!IsPathSafe(file_path)) {
                LogWarn("Unsafe path detected: %s", file_path.c_str());
                sp_ctx->res().status_code = StatusCode::k403_Forbidden;
                return;
            }

            auto file_type = util::fs::GetFileType(file_path);
            //! 检查路径是否是目录
            if (file_type == util::fs::FileType::kDirectory) {
                //! 如果是目录且路径不以'/'结尾，进行重定向
                if (!request_path.empty() && request_path.back() != '/') {
                    sp_ctx->res().status_code = StatusCode::k301_MovedPermanently;
                    sp_ctx->res().headers["Location"] = request_path + "/";
                    return;
                }

                //! 尝试访问默认文件
                std::string default_file_path = file_path + dir.default_file;
                if (util::fs::GetFileType(default_file_path) == util::fs::FileType::kRegular) {
                    if (respondFile(sp_ctx, default_file_path))
                        return;
                }

                //! 如果允许目录列表，生成目录内容
                if (d_->directory_listing_enabled) {
                    if (respondDirectory(sp_ctx, file_path, request_path))
                        return;
                }

                //! 否则返回403 Forbidden
                LogNotice("Directory listing disabled for: %s", file_path.c_str());
                sp_ctx->res().status_code = StatusCode::k403_Forbidden;
                return;

            } else if (file_type == util::fs::FileType::kRegular) {
                //! 如果是普通文件，直接响应文件内容
                if (respondFile(sp_ctx, file_path))
                    return;
            }
        }
    }

    //! 如果没有找到匹配的文件，传递给下一个中间件
    next();
}

std::string FileDownloaderMiddleware::getMimeType(const std::string& filename) const
{
    //! 查找最后一个点的位置
    size_t dot_pos = filename.find_last_of('.');
    if (dot_pos != std::string::npos) {
        std::string ext = util::string::ToLower(filename.substr(dot_pos + 1));
        //! 在MIME类型映射中查找
        auto it = d_->mime_types.find(ext);
        if (it != d_->mime_types.end())
            return it->second;
    }

    //! 未找到匹配的MIME类型，返回默认值
    return d_->default_mime_type;
}

bool FileDownloaderMiddleware::respondFile(ContextSptr sp_ctx, const std::string& file_path)
{
    auto& res = sp_ctx->res();

    //! 用 stat() 获取文件元信息，同时验证文件是否存在
    struct stat file_stat;
    if (::stat(file_path.c_str(), &file_stat) != 0) {
        res.status_code = StatusCode::k404_NotFound;
        return true;
    }

    size_t file_size  = static_cast<size_t>(file_stat.st_size);
    time_t file_mtime = file_stat.st_mtime;
    std::string etag          = GenerateETag(file_mtime, file_stat.st_size);
    std::string last_modified = HttpDateToString(file_mtime);

    res.headers["Content-Type"]  = getMimeType(file_path);
    res.headers["Accept-Ranges"] = "bytes";
    res.headers["ETag"]          = etag;
    res.headers["Last-Modified"] = last_modified;
    res.headers["Cache-Control"] = "public, max-age=0, must-revalidate";
    res.headers["Access-Control-Allow-Origin"]   = "*";
    res.headers["Access-Control-Expose-Headers"] = "Content-Range, Content-Length, ETag, Last-Modified";

    //! 条件请求：If-None-Match 优先于 If-Modified-Since（RFC 7232 §6）
    std::string if_none_match = GetHeader(sp_ctx->req().headers, "if-none-match");
    if (!if_none_match.empty()) {
        if (if_none_match == etag) {
            res.status_code = StatusCode::k304_NotModified;
            return true;
        }
    } else {
        std::string if_modified_since = GetHeader(sp_ctx->req().headers, "if-modified-since");
        if (!if_modified_since.empty()) {
            time_t since = StringToHttpDate(if_modified_since);
            if (since != -1 && file_mtime <= since) {
                res.status_code = StatusCode::k304_NotModified;
                return true;
            }
        }
    }

    //! 解析 Range 请求头（需在 HEAD 判断之前，确保非法 Range 返回 416）
    size_t range_begin = 0;
    size_t range_end   = file_size > 0 ? file_size - 1 : 0;
    bool   has_range   = false;

    if (file_size > 0) {
        auto range_str = GetHeader(sp_ctx->req().headers, "range");
        if (util::string::IsStartWith(range_str, "bytes=")) {
            has_range = ParseRangeString(range_str, file_size, range_begin, range_end);
        }

        //! If-Range：ETag 不匹配时降级为全量响应，保证数据一致性
        if (has_range) {
            std::string if_range = GetHeader(sp_ctx->req().headers, "if-range");
            if (!if_range.empty() && if_range != etag) {
                has_range   = false;
                range_begin = 0;
                range_end   = file_size - 1;
            }
        }

        //! 校验范围合法性
        if (has_range && (range_begin >= file_size || range_end >= file_size || range_begin > range_end)) {
            res.status_code = StatusCode::k416_RequestedRangeNotSatisfiable;
            res.headers["Content-Range"] = "bytes */" + std::to_string(file_size);
            return true;
        }
    }

    size_t content_length = file_size > 0 ? range_end - range_begin + 1 : 0;
    res.headers["Content-Length"] = std::to_string(content_length);

    if (has_range) {
        res.status_code = StatusCode::k206_PartialContent;
        res.headers["Content-Range"] = "bytes " + std::to_string(range_begin) + "-" +
            std::to_string(range_end) + "/" + std::to_string(file_size);
    } else {
        res.status_code = StatusCode::k200_OK;
    }

    //! HEAD 请求不返回 body
    if (sp_ctx->req().method == Method::kHead)
        return true;

    //! 空文件直接返回
    if (file_size == 0)
        return true;

    if (content_length < d_->switch_to_worker_filesize_threshold) {
        std::ifstream file(file_path, std::ios::binary);
        if (!file.is_open()) {
            res.status_code = StatusCode::k500_InternalServerError;
            return true;
        }
        file.seekg(static_cast<std::streamoff>(range_begin));
        res.body.resize(content_length);
        file.read(&res.body[0], static_cast<std::streamsize>(content_length));
        LogInfo("Served file: %s (bytes %zu-%zu/%zu)",
                file_path.c_str(), range_begin, range_end, file_size);
    } else {
        d_->worker->execute(
            [sp_ctx, file_path, range_begin, content_length] {
                auto& res = sp_ctx->res();
                std::ifstream f(file_path, std::ios::binary);
                if (f.is_open()) {
                    f.seekg(static_cast<std::streamoff>(range_begin));
                    res.body.resize(content_length);
                    f.read(&res.body[0], static_cast<std::streamsize>(content_length));
                    LogInfo("Served file(worker): %s (%zu bytes from %zu)",
                            file_path.c_str(), content_length, range_begin);
                } else {
                    res.status_code = StatusCode::k500_InternalServerError;
                }
            },
            [sp_ctx] { } //! 确保 sp_ctx 在主线程上析构
         );
    }

    return true;
}

bool FileDownloaderMiddleware::respondDirectory(ContextSptr sp_ctx,
                                                const std::string& dir_path,
                                                const std::string& url_path)
{
    try {
        //! 生成HTML目录列表
        std::ostringstream html_oss;
        html_oss
            << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "<head>\n"
            << "  <title>Directory listing for " << url_path << "</title>\n"
            << "  <style>\n"
            << "    body { font-family: Arial, sans-serif; margin: 20px; }\n"
            << "    h1 { color: #333; }\n"
            << "    ul { list-style-type: none; padding: 0; }\n"
            << "    li { margin: 5px 0; }\n"
            << "    a { color: #0066cc; text-decoration: none; }\n"
            << "    a:hover { text-decoration: underline; }\n"
            << "    .dir { font-weight: bold; }\n"
            << "  </style>\n"
            << "</head>\n"
            << "<body>\n"
            << "  <h1>Directory listing for " << url_path << "</h1>\n"
            << "  <ul>\n";

        //! 如果不是根目录，添加返回上级目录的链接
        if (url_path != "/") {
            size_t last_slash = url_path.find_last_of('/', url_path.size() - 2);
            if (last_slash != std::string::npos) {
                std::string parent_url = url_path.substr(0, last_slash + 1);
                html_oss << "    <li><a href=\"" << parent_url << "\">..</a></li>\n";
            }
        }

        //! 列出目录中的项目
        std::vector<std::string> entries;
        if (!util::fs::ListDirectory(dir_path, entries)) {
            LogErr("Failed to list directory: %s", dir_path.c_str());
            return false;
        }

        for (const auto& name : entries) {
            std::string entry_path = dir_path + "/" + name;
            std::string href = url_path + name;

            auto entry_type = util::fs::GetFileType(entry_path);
            if (entry_type == util::fs::FileType::kDirectory) {
                href += "/";
                html_oss << "    <li><a class=\"dir\" href=\"" << href << "\">" << name << "/</a></li>\n";
            } else {
                html_oss << "    <li><a href=\"" << href << "\">" << name << "</a></li>\n";
            }
        }

        html_oss
            << "  </ul>\n"
            << "</body>\n"
            << "</html>";

        //! 设置响应
        auto& res = sp_ctx->res();
        res.status_code = StatusCode::k200_OK;
        res.headers["Content-Type"] = "text/html; charset=utf-8";
        res.body = html_oss.str();

        LogInfo("Served directory listing for: %s", dir_path.c_str());
        return true;

    } catch (const std::exception& e) {
        LogErr("Failed to generate directory listing: %s", e.what());
        return false;
    }
}

}
}
}
