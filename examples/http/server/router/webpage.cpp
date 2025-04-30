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

namespace webpage {

const char* kHtmlHome =
R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>TBOX HTTP 示例</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 2rem;
            background-color: #f7f9fc;
        }
        .header {
            text-align: center;
            margin-bottom: 2rem;
            padding-bottom: 1rem;
            border-bottom: 1px solid #eaeaea;
        }
        h1 {
            color: #2c3e50;
        }
        .card-container {
            display: flex;
            gap: 1.5rem;
            flex-wrap: wrap;
            justify-content: center;
        }
        .card {
            background: white;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            padding: 1.5rem;
            width: 200px;
            transition: transform 0.3s ease, box-shadow 0.3s ease;
        }
        .card:hover {
            transform: translateY(-5px);
            box-shadow: 0 10px 20px rgba(0, 0, 0, 0.1);
        }
        .card h2 {
            color: #3498db;
            margin-top: 0;
        }
        .card p {
            color: #7f8c8d;
        }
        .btn {
            display: inline-block;
            background-color: #3498db;
            color: white;
            text-decoration: none;
            padding: 0.5rem 1rem;
            border-radius: 4px;
            font-weight: 500;
            transition: background-color 0.3s;
        }
        .btn:hover {
            background-color: #2980b9;
        }
        .footer {
            margin-top: 3rem;
            text-align: center;
            font-size: 0.9rem;
            color: #95a5a6;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>TBOX HTTP 示例服务器</h1>
        <p>这是一个简单的C++ HTTP服务器演示</p>
    </div>

    <div class="card-container">
        <div class="card">
            <h2>页面一</h2>
            <p>演示第一个示例页面</p>
            <a href="/1" class="btn">访问</a>
        </div>

        <div class="card">
            <h2>页面二</h2>
            <p>演示第二个示例页面</p>
            <a href="/2" class="btn">访问</a>
        </div>
    </div>

    <div class="footer">
        <p>Powered by TBOX Framework - &copy; 2023</p>
    </div>
</body>
</html>
)";

const char* kHtmlPage1 =
 R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>页面一 - TBOX HTTP 示例</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 2rem;
            background-color: #e8f4f8;
        }
        .container {
            background: white;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            padding: 2rem;
        }
        h1 {
            color: #2980b9;
            border-bottom: 2px solid #3498db;
            padding-bottom: 0.5rem;
        }
        .content {
            margin-top: 1.5rem;
        }
        .back {
            display: inline-block;
            margin-top: 2rem;
            background-color: #3498db;
            color: white;
            text-decoration: none;
            padding: 0.5rem 1rem;
            border-radius: 4px;
        }
        .back:hover {
            background-color: #2980b9;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>页面一</h1>
        <div class="content">
            <p>这是第一个示例页面，展示了TBOX HTTP服务器的基本路由功能。</p>
            <p>您可以根据需要自由扩展这个页面的内容和样式。</p>
        </div>
        <a href="/" class="back">返回首页</a>
    </div>
</body>
</html>)";

const char* kHtmlPage2 =
R"(<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>页面二 - TBOX HTTP 示例</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            line-height: 1.6;
            color: #333;
            max-width: 800px;
            margin: 0 auto;
            padding: 2rem;
            background-color: #f0f4e8;
        }
        .container {
            background: white;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            padding: 2rem;
        }
        h1 {
            color: #27ae60;
            border-bottom: 2px solid #2ecc71;
            padding-bottom: 0.5rem;
        }
        .content {
            margin-top: 1.5rem;
        }
        .feature-list {
            margin: 1.5rem 0;
        }
        .feature-list li {
            margin-bottom: 0.5rem;
        }
        .back {
            display: inline-block;
            margin-top: 2rem;
            background-color: #27ae60;
            color: white;
            text-decoration: none;
            padding: 0.5rem 1rem;
            border-radius: 4px;
        }
        .back:hover {
            background-color: #219653;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>页面二</h1>
        <div class="content">
            <p>欢迎来到第二个示例页面！这里展示了更多的HTML内容。</p>

            <div class="feature-list">
                <h3>TBOX框架特点：</h3>
                <ul>
                    <li>高性能的事件循环</li>
                    <li>简洁的HTTP服务器API</li>
                    <li>灵活的中间件系统</li>
                    <li>易于集成的模块化设计</li>
                </ul>
            </div>
        </div>
        <a href="/" class="back">返回首页</a>
    </div>
</body>
</html>)";

}
