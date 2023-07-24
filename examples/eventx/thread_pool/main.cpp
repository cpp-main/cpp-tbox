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
#include <string>
#include <thread>   //! 引用std::this_thread::sleep_for
#include <memory>   //! 引入智能指针

#include <tbox/event/loop.h>
#include <tbox/eventx/thread_pool.h>
#include <tbox/base/log.h>
#include <tbox/base/log_output.h>

using namespace std;
using namespace tbox::event;
using namespace tbox::eventx;

//! 模拟存储大数据到文件
int StoreDataToFile(const string &filename, const string &content)
{
    LogInfo("Writing to %s, size:%d ...", filename.c_str(), content.size());
    std::this_thread::sleep_for(std::chrono::seconds(1));
    LogInfo("Done");
    return 0;
}

int main(int argc, char **argv)
{
    LogOutput_Enable();

    Loop* sp_loop = Loop::New();
    ThreadPool *sp_tp = new ThreadPool(sp_loop);
    sp_tp->initialize();

    struct StoreDataToFileTask {
        string filename;
        string content;
        int ret = -1;
    };

    sp_loop->runInLoop(
        [=] {
            //! 准备任务明细
            auto sp_data = std::make_shared<StoreDataToFileTask>();
            sp_data->filename = "/tmp/test.txt";
            sp_data->content = "this is a test of threadpool";

            LogInfo("Before commit task");
            //! 向线程池提交任务
            sp_tp->execute(
                [sp_data]{  //! 指定任务线程要做的事情
                    sp_data->ret = StoreDataToFile(sp_data->filename, sp_data->content);
                },
                [sp_data, sp_loop]{ //! 指定任务线程完成了任务后主线程接下来要做的事情
                    LogInfo("ret:%d", sp_data->ret);
                    sp_loop->exitLoop();
                }
            );
            LogInfo("After commit task");
        }
    );

    LogInfo("Start");
    sp_loop->runLoop();
    LogInfo("Stoped");

    sp_tp->cleanup();

    delete sp_tp;
    delete sp_loop;
    return 0;
}
