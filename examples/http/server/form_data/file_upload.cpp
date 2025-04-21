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
 * 文件上传服务器示例
 *
 * 这个示例展示了如何使用 FormDataMiddleware 中间件处理文件上传
 */

#include <signal.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/base/scope_exit.hpp>
#include <tbox/event/signal_event.h>
#include <tbox/event/loop.h>
#include <tbox/eventx/work_thread.h>
#include <tbox/network/sockaddr.h>
#include <tbox/http/server/server.h>
#include <tbox/http/server/middlewares/router_middleware.h>
#include <tbox/http/server/middlewares/form_data_middleware.h>
#include <tbox/util/fs.h>

using namespace std;
using namespace tbox;
using namespace tbox::event;
using namespace tbox::network;
using namespace tbox::http;
using namespace tbox::http::server;

// HTML表单页面
const char *kHtmlForm = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>文件上传示例</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #333; }
        form { border: 1px solid #ddd; padding: 20px; border-radius: 5px; max-width: 500px; }
        .form-group { margin-bottom: 15px; }
        label { display: block; margin-bottom: 5px; font-weight: bold; }
        input[type="text"], input[type="email"] { width: 100%; padding: 8px; box-sizing: border-box; }
        input[type="submit"] { background-color: #4CAF50; color: white; padding: 10px 15px; border: none; cursor: pointer; }
        input[type="submit"]:hover { background-color: #45a049; }
    </style>
</head>
<body>
    <h1>文件上传示例</h1>
    <form action="/upload" method="post" enctype="multipart/form-data">
        <div class="form-group">
            <label for="name">姓名</label>
            <input type="text" id="name" name="name" required>
        </div>
        <div class="form-group">
            <label for="email">邮箱</label>
            <input type="email" id="email" name="email" required>
        </div>
        <div class="form-group">
            <label for="file">选择文件</label>
            <input type="file" id="file" name="file" required>
        </div>
        <div class="form-group">
            <label for="description">描述</label>
            <textarea id="description" name="description" rows="4" style="width: 100%; padding: 8px; box-sizing: border-box;"></textarea>
        </div>
        <input type="submit" value="上传">
    </form>
</body>
</html>
)";

// 显示成功页面
std::string GenSuccessHtml(const std::string& name, const std::string& email, const std::string& filename, const std::string& description) {
    std::string html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>上传成功</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        h1 { color: #4CAF50; }
        .info { border: 1px solid #ddd; padding: 20px; border-radius: 5px; max-width: 500px; }
        .label { font-weight: bold; }
        .back { margin-top: 20px; }
        .back a { color: #4CAF50; text-decoration: none; }
    </style>
</head>
<body>
    <h1>文件上传成功！</h1>
    <div class="info">
        <p><span class="label">姓名:</span>)";
    html += name;
    html += R"(</p>
        <p><span class="label">邮箱:</span>)";
    html += email;
    html += R"(</p>
        <p><span class="label">文件名:</span>)";
    html += filename;
    html += R"(</p>
        <p><span class="label">描述:</span>)";
    html += description;
    html += R"(</p>
    </div>
    <div class="back">
        <a href="/">返回上传页面</a>
    </div>
</body>
</html>
    )";
    return html;
}

int main(int argc, char **argv)
{
    std::string bind_addr = "0.0.0.0:8080";
    std::string upload_dir = "./uploads";

    if (argc >= 2)
        bind_addr = argv[1];

    if (argc >= 3)
        upload_dir = argv[2];

    // 启用日志输出
    LogOutput_Enable();
    LogInfo("Starting file upload server");

    // 创建上传目录
    if (!tbox::util::fs::MakeDirectory(upload_dir)) {
        LogErr("Cannot create upload directory, exiting");
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

    tbox::eventx::WorkThread worker(sp_loop);

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

    // 创建路由和表单数据处理器
    RouterMiddleware router;
    FormDataMiddleware form_data;

    // 添加首页路由
    router.get("/", [](ContextSptr ctx, const NextFunc& next) {
        ctx->res().status_code = StatusCode::k200_OK;
        ctx->res().headers["Content-Type"] = "text/html; charset=UTF-8";
        ctx->res().body = kHtmlForm;
    });

    // 添加文件上传处理
    form_data.post("/upload", [&upload_dir, &worker] (ContextSptr ctx, const FormData& form_data, const NextFunc& next) {
        // 获取表单字段
        std::string name, email, description, filename;
        auto sp_file_content = std::make_shared<std::string>();

        if (!form_data.getField("name", name))
            name = "未提供";

        if (!form_data.getField("email", email))
            email = "未提供";

        if (!form_data.getField("description", description))
            description = "未提供";

        // 获取上传的文件
        if (form_data.getFile("file", filename, *sp_file_content)) {
            // 保存文件
            if (!filename.empty()) {
                worker.execute(
                    [ctx, upload_dir, name, email, filename, description, sp_file_content] {
                        std::string file_path = upload_dir + "/" + filename;
                        if (tbox::util::fs::WriteBinaryToFile(file_path, *sp_file_content)) {
                            // 返回成功页面
                            ctx->res().status_code = StatusCode::k200_OK;
                            ctx->res().headers["Content-Type"] = "text/html; charset=UTF-8";
                            ctx->res().body = GenSuccessHtml(name, email, filename, description);
                        } else {
                            LogWarn("Cannot create file: %s", file_path.c_str());
                            ctx->res().status_code = StatusCode::k500_InternalServerError;
                            ctx->res().body = "Server Error: Cannot save file";
                        }
                    },
                    [ctx] { }
                );
            } else {
                LogNotice("Filename is empty");
                ctx->res().status_code = StatusCode::k400_BadRequest;
                ctx->res().body = "Error: No filename provided";
            }
        } else {
            LogNotice("File not found");
            ctx->res().status_code = StatusCode::k400_BadRequest;
            ctx->res().body = "Error: No file uploaded";
        }
    });

    // 添加中间件到服务器
    srv.use(&router);
    srv.use(&form_data);

    // 设置信号处理回调
    sp_sig_event->setCallback(
        [&] (int sig) {
            LogInfo("Received signal %d, preparing to exit...", sig);
            srv.stop();
            sp_loop->exitLoop();
        }
    );

    LogInfo("File upload server started, listening on http://%s/", bind_addr.c_str());

    // 运行事件循环
    sp_loop->runLoop();

    LogInfo("Server stopped");
    srv.cleanup();

    LogInfo("Exiting");
    return 0;
}
