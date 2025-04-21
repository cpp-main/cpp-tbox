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

/**
 * 文件下载服务器示例
 *
 * 这个示例展示了如何使用 FileDownloaderMiddleware 实现文件下载功能
 * 包含目录浏览和文件下载功能
 */

#include <signal.h>
#include <iostream>
#include <filesystem>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/signal_event.h>
#include <tbox/event/loop.h>
#include <tbox/network/sockaddr.h>
#include <tbox/http/server/server.h>
#include <tbox/http/server/middlewares/router_middleware.h>
#include <tbox/http/server/middlewares/file_downloader_middleware.h>
#include <tbox/util/fs.h>
#include <tbox/trace/sink.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;
using namespace tbox::http;
using namespace tbox::http::server;

// 文件下载服务器状态页面
const char *kStatusHtml = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>文件下载服务器</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        .status { border: 1px solid #ddd; padding: 20px; border-radius: 5px; max-width: 500px; }
        .info { margin-bottom: 10px; }
        .label { font-weight: bold; }
        .good { color: #4CAF50; }
        .links { margin-top: 20px; }
        .links a { color: #4CAF50; text-decoration: none; margin-right: 15px; }
    </style>
</head>
<body>
    <h1>文件下载服务器</h1>
    <div class="status">
        <div class="info"><span class="label">状态:</span> <span class="good">运行中</span></div>
        <div class="info"><span class="label">版本:</span> 1.0.0</div>
        <div class="info"><span class="label">目录浏览:</span> 已启用</div>
    </div>
    <div class="links">
        <a href="/">浏览文件</a>
        <a href="/api/status">API状态</a>
    </div>
</body>
</html>
)";

int main(int argc, char **argv)
{
    std::string serve_dir = "./";
    std::string bind_addr = "0.0.0.0:8080";

    // 处理命令行参数
    if (argc >= 2)
        serve_dir = argv[1];

    if (argc >= 3)
        bind_addr = argv[2];

    // 启用日志输出
    LogOutput_Enable();
    LogInfo("Starting file download server");

#if 0   //! 要监控性能时，打开
    auto &trace_sink = tbox::trace::Sink::GetInstance();
    trace_sink.setPathPrefix("/tmp/file_download/trace");
    trace_sink.enable();
#endif

    // 检查目录是否存在
    if (!tbox::util::fs::IsDirectoryExist(serve_dir)) {
        LogErr("Directory '%s' doesn't exist or is not a directory", serve_dir.c_str());
        return 1;
    }

    // 创建事件循环和信号处理
    auto sp_loop = Loop::New();
    auto sp_sig_event = sp_loop->newSignalEvent();

    // 确保资源被正确释放
    SetScopeExitAction(
        [=] {
            delete sp_sig_event;
            delete sp_loop;
        }
    );

    // 初始化信号处理
    sp_sig_event->initialize({SIGINT, SIGTERM}, Event::Mode::kPersist);
    sp_sig_event->enable();

    // 创建HTTP服务器
    Server srv(sp_loop);
    if (!srv.initialize(SockAddr::FromString(bind_addr), 5)) {
        LogErr("HTTP server initialization failed");
        return 1;
    }

    srv.start();
    //srv.setContextLogEnable(true);    //! 调试时需要看详细收发数据时可以打开

    // 创建路由器和文件下载中间件
    RouterMiddleware router;
    FileDownloaderMiddleware file_downloader(sp_loop);

    // 添加状态页面路由
    router.get("/api/status", [](ContextSptr ctx, const NextFunc& next) {
        ctx->res().status_code = StatusCode::k200_OK;
        ctx->res().headers["Content-Type"] = "application/json";
        ctx->res().body = R"({"status":"running","version":"1.0.0"})";
    });

    router.get("/status", [](ContextSptr ctx, const NextFunc& next) {
        ctx->res().status_code = StatusCode::k200_OK;
        ctx->res().headers["Content-Type"] = "text/html; charset=UTF-8";
        ctx->res().body = kStatusHtml;
    });

    // 配置文件下载中间件
    if (!file_downloader.addDirectory("/", serve_dir)) {
        LogErr("Failed to add directory: %s", serve_dir.c_str());
        return 1;
    }

    // 启用目录浏览功能
    file_downloader.setDirectoryListingEnabled(true);

    // 添加常见的 MIME 类型映射（如果需要额外的映射）
    // file_downloader.setMimeType("pdf", "application/pdf");

    // 添加中间件到服务器
    srv.use(&router);
    srv.use(&file_downloader);

    // 设置信号处理回调
    sp_sig_event->setCallback(
        [&] (int sig) {
            LogInfo("Received signal %d, preparing to exit...", sig);
            srv.stop();
            sp_loop->exitLoop();
        }
    );

    LogInfo("File download server started, listening on http://%s/, serving directory: %s",
            bind_addr.c_str(), serve_dir.c_str());

    // 运行事件循环
    sp_loop->runLoop();

    LogInfo("Server stopped");
    srv.cleanup();

    LogInfo("Exiting");
    return 0;
}
