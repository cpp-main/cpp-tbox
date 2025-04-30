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
#ifndef TBOX_HTTP_SERVER_FILE_DOWNLOADER_MIDDLEWARE_H_20250419
#define TBOX_HTTP_SERVER_FILE_DOWNLOADER_MIDDLEWARE_H_20250419

#include <string>
#include <vector>
#include <map>

#include <tbox/base/defines.h>
#include <tbox/event/forward.h>

#include "../middleware.h"
#include "../context.h"

namespace tbox {
namespace http {
namespace server {

/**
 * 文件下载中间件
 *
 * 用于处理静态文件的下载请求，可以设置多个目录作为文件源
 * 自动防止目录遍历攻击（防止访问指定目录之外的文件）
 */
class FileDownloaderMiddleware : public Middleware {
  public:
    /**
     * 构造函数
     */
    explicit FileDownloaderMiddleware(event::Loop *wp_loop);
    virtual ~FileDownloaderMiddleware();

    NONCOPYABLE(FileDownloaderMiddleware);

    /**
     * 添加文件目录
     *
     * \param url_prefix   URL前缀，以'/'开头
     * \param local_path   本地文件目录的路径，可以是相对路径或绝对路径
     * \param default_file 默认文件，当请求目录时返回的文件，默认是"index.html"
     *
     * \return 是否添加成功
     */
    bool addDirectory(const std::string& url_prefix,
                      const std::string& local_path,
                      const std::string& default_file = "index.html");

    /**
     * 设置是否允许列出目录内容
     *
     * \param enable 是否启用列目录功能
     */
    void setDirectoryListingEnabled(bool enable);

    /**
     * 设置路径映射，允许将特定URL映射到特定文件
     *
     * \param url   URL路径
     * \param file  文件路径
     */
    void setPathMapping(const std::string& url, const std::string& file);

    /**
     * 设置默认的MIME类型
     *
     * \param mime_type MIME类型字符串
     */
    void setDefaultMimeType(const std::string& mime_type);

    /**
     * 设置文件扩展名到MIME类型的映射
     *
     * \param ext       文件扩展名（不包含'.'）
     * \param mime_type MIME类型字符串
     */
    void setMimeType(const std::string& ext, const std::string& mime_type);

  protected:
    /**
     * 实现Middleware接口的处理函数
     */
    virtual void handle(ContextSptr sp_ctx, const NextFunc& next) override;

  private:
    /**
     * 根据文件扩展名获取MIME类型
     */
    std::string getMimeType(const std::string& filename) const;

    /**
     * 响应文件内容
     */
    bool respondFile(ContextSptr sp_ctx, const std::string& file_path);

    /**
     * 生成目录列表
     */
    bool respondDirectory(ContextSptr sp_ctx, const std::string& dir_path, const std::string& url_path);

  private:
    struct Data;
    Data* d_;
};

}
}
}

#endif // TBOX_HTTP_SERVER_FILE_DOWNLOADER_MIDDLEWARE_H_20250419
