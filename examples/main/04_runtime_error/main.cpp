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
#include <thread>
#include <tbox/main/main.h>
#include <tbox/terminal/session.h>

namespace tbox {
namespace main {

class TestModule : public Module {
  public:
    TestModule(Context &ctx) :
        Module("test", ctx)
    { }

    virtual bool onInit(const Json &) {
        using namespace terminal;
        auto &shell = *ctx().terminal();
        auto root_node = shell.rootNode();

        {
            auto func_node = shell.createFuncNode(
                [] (const Session &, const Args &) {
                    static_cast<char*>(nullptr)[0] = 0;
                }
            );
            shell.mountNode(root_node, func_node, "crash");
        }
        {
            auto func_node = shell.createFuncNode(
                [this] (const Session &, const Args &) {
                    ctx().thread_pool()->execute(
                        [] { static_cast<char*>(nullptr)[0] = 0; }
                    );
                }
            );
            shell.mountNode(root_node, func_node, "crash_thread");
        }

        {
            auto func_node = shell.createFuncNode(
                [] (const Session &, const Args &) {
                    throw 10;
                }
            );
            shell.mountNode(root_node, func_node, "throw_int");
        }
        {
            auto func_node = shell.createFuncNode(
                [] (const Session &, const Args &) {
                    throw std::runtime_error("runtime error");
                }
            );
            shell.mountNode(root_node, func_node, "throw_runtime_error");
        }
        {
            auto func_node = shell.createFuncNode(
                [this] (const Session &, const Args &) {
                    ctx().thread_pool()->execute(
                        [] { throw 10; }
                    );
                }
            );
            shell.mountNode(root_node, func_node, "throw_int_thread");
        }
        {
            auto func_node = shell.createFuncNode(
                [this] (const Session &, const Args &) {
                    ctx().thread_pool()->execute(
                        [] { throw std::runtime_error("runtime error"); }
                    );
                }
            );
            shell.mountNode(root_node, func_node, "throw_runtime_error_thread");
        }
        {
            auto func_node = shell.createFuncNode(
                [] (const Session &, const Args &) {
                    std::this_thread::sleep_for(std::chrono::seconds(3));
                }
            );
            shell.mountNode(root_node, func_node, "block");
        }

        return true;
    }
};

void RegisterApps(Module &apps, Context &ctx)
{
    apps.add(new TestModule(ctx));
}

std::string GetAppDescribe()
{
    return "One app sample";
}

std::string GetAppBuildTime()
{
    return __DATE__ " " __TIME__;
}

void GetAppVersion(int &major, int &minor, int &rev, int &build)
{
    major = 0;
    minor = 0;
    rev = 2;
    build = 0;
}

}
}
