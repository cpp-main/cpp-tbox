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

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/util/string.h>
#include <tbox/util/fs.h>
#include <tbox/eventx/work_thread.h>
#include <tbox/base/recorder.h>

namespace tbox {
namespace http {
namespace server {

namespace {
bool IsPathSafe(const std::string& path) {
    //! 检查是否有".."路径组件，这可能导致目录遍历
    std::istringstream path_stream(path);
    std::string component;

    while (std::getline(path_stream, component, '/')) {
        if (component == "..")
            return false; //! 不允许上级目录访问
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
    eventx::WorkThread worker;
    std::vector<DirectoryConfig> directories;        //! 目录配置列表
    std::map<std::string, std::string> path_mappings;//! 特定路径映射
    std::map<std::string, std::string> mime_types;   //! MIME类型映射
    std::string default_mime_type;                   //! 默认MIME类型
    bool directory_listing_enabled;                  //! 是否允许目录列表
    size_t switch_to_worker_filesize_threshold;

    Data(event::Loop *wp_loop)
        : worker(wp_loop)
        , default_mime_type("application/octet-stream")
        , directory_listing_enabled(false)
        , switch_to_worker_filesize_threshold(100 << 10)
    {
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
};

FileDownloaderMiddleware::FileDownloaderMiddleware(event::Loop *wp_loop)
    : d_(new Data(wp_loop))
{ }

FileDownloaderMiddleware::~FileDownloaderMiddleware() { delete d_; }

bool FileDownloaderMiddleware::addDirectory(const std::string& url_prefix,
                                            const std::string& local_path,
                                            const std::string& default_file) {
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

void FileDownloaderMiddleware::setDirectoryListingEnabled(bool enable) {
    d_->directory_listing_enabled = enable;
}

void FileDownloaderMiddleware::setPathMapping(const std::string& url, const std::string& file) {
    d_->path_mappings[url] = file;
}

void FileDownloaderMiddleware::setDefaultMimeType(const std::string& mime_type) {
    d_->default_mime_type = mime_type;
}

void FileDownloaderMiddleware::setMimeType(const std::string& ext, const std::string& mime_type) {
    d_->mime_types[ext] = mime_type;
}

void FileDownloaderMiddleware::handle(ContextSptr sp_ctx, const NextFunc& next) {
    const auto& request = sp_ctx->req();

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
        if (request_path.find(dir.url_prefix) == 0) {
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

std::string FileDownloaderMiddleware::getMimeType(const std::string& filename) const {
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

bool FileDownloaderMiddleware::respondFile(ContextSptr sp_ctx, const std::string& file_path) {
    auto& res = sp_ctx->res();

    //! 打开文件
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        res.status_code = StatusCode::k404_NotFound;
        return true;
    }

    res.headers["Content-Type"] = getMimeType(file_path);

    //! 获取文件大小
    size_t file_size = static_cast<size_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    //! 如果是HEAD请求，不返回内容
    if (sp_ctx->req().method == Method::kHead) {
        res.status_code = StatusCode::k200_OK;
        res.headers["Content-Length"] = std::to_string(file_size);
        return true;
    }

    //! 文件是否大于100KB
    if (file_size < d_->switch_to_worker_filesize_threshold) {
        //! 小文件就直接读了
        res.status_code = StatusCode::k200_OK;
        res.headers["Content-Length"] = std::to_string(file_size);
        //! 将文件内容读到body中去
        res.body = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        LogInfo("Served file: %s (%zu bytes)", file_path.c_str(), file_size);

    } else {
        //! 文件太大就采用子线程去读
        d_->worker.execute(
            [sp_ctx, file_path] {
                auto& res = sp_ctx->res();
                if (util::fs::ReadBinaryFromFile(file_path, res.body)) {
                    res.status_code = StatusCode::k200_OK;
                    res.headers["Content-Length"] = std::to_string(res.body.size());
                    LogInfo("Served file: %s (%zu bytes)", file_path.c_str(), res.body.size());
                } else {
                    res.status_code = StatusCode::k404_NotFound;
                }
            },
            [sp_ctx] { } //! 这是为了确保sp_ctx在主线程上析构
        );
    }

    return true;
}

bool FileDownloaderMiddleware::respondDirectory(ContextSptr sp_ctx,
                                                const std::string& dir_path,
                                                const std::string& url_path) {
    try {
        //! 生成HTML目录列表
        std::stringstream html;
        html << "<!DOCTYPE html>\n"
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
                html << "    <li><a href=\"" << parent_url << "\">..</a></li>\n";
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
                html << "    <li><a class=\"dir\" href=\"" << href << "\">" << name << "/</a></li>\n";
            } else {
                html << "    <li><a href=\"" << href << "\">" << name << "</a></li>\n";
            }
        }

        html << "  </ul>\n"
             << "</body>\n"
             << "</html>";

        //! 设置响应
        auto& res = sp_ctx->res();
        res.status_code = StatusCode::k200_OK;
        res.headers["Content-Type"] = "text/html; charset=utf-8";
        res.body = html.str();

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
