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
#include "async.h"

#include <memory>
#include <tbox/base/assert.h>
#include <tbox/util/fs.h>
#include <tbox/util/execute_cmd.h>

namespace tbox {
namespace eventx {

Async::Async(eventx::ThreadPool *thread_pool)
    : thread_pool_(thread_pool)
{ }

void Async::readFile(const std::string &filename, StringCallback &&cb)
{
    TBOX_ASSERT(!filename.empty());
    TBOX_ASSERT(cb != nullptr);

    struct Tmp {
        int error_code = 0;
        std::string result;
        StringCallback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
        [tmp, filename] {
            if (!util::fs::ReadStringFromTextFile(filename, tmp->result))
                tmp->error_code = 1;
        },
        [tmp] {
            tmp->cb(tmp->error_code, tmp->result);
        }
    );
}

void Async::readFileLines(const std::string &filename, StringVecCallback &&cb)
{
    TBOX_ASSERT(!filename.empty());
    TBOX_ASSERT(cb != nullptr);

    struct Tmp {
        int error_code = 0;
        std::vector<std::string> line_vec;
        StringVecCallback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
        [tmp, filename] {
            auto is_succ = util::fs::ReadEachLineFromTextFile(filename,
                [tmp](const std::string &line) {
                    tmp->line_vec.emplace_back(line);
                }
            );

            if (!is_succ)
                tmp->error_code = 1;
        },
        [tmp] {
            tmp->cb(tmp->error_code, tmp->line_vec);
        }
    );
}

void Async::writeFile(const std::string &filename, const std::string &content, bool sync_now, Callback &&cb)
{
    TBOX_ASSERT(!filename.empty());

    struct Tmp {
        int error_code = 0;
        Callback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
        [tmp, filename, content, sync_now] {
            if (!util::fs::WriteStringToTextFile(filename, content, sync_now))
                tmp->error_code = 1;
        },
        [tmp] {
            if (tmp->cb)
                tmp->cb(tmp->error_code);
        }
    );
}

void Async::appendFile(const std::string &filename, const std::string &content, bool sync_now, Callback &&cb)
{
    TBOX_ASSERT(!filename.empty());

    struct Tmp {
        int error_code = 0;
        Callback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
        [tmp, filename, content, sync_now] {
            if (!util::fs::AppendStringToTextFile(filename, content, sync_now))
                tmp->error_code = 1;
        },
        [tmp] {
            if (tmp->cb)
                tmp->cb(tmp->error_code);
        }
    );
}

void Async::removeFile(const std::string &filename, Callback &&cb)
{
    TBOX_ASSERT(!filename.empty());

    struct Tmp {
        int error_code = 0;
        Callback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
        [tmp, filename] {
            if (!util::fs::RemoveFile(filename))
                tmp->error_code = 1;
        },
        [tmp] {
            if (tmp->cb)
                tmp->cb(tmp->error_code);
        }
    );
}

void Async::executeCmd(const std::string &cmd, Callback &&cb)
{
    TBOX_ASSERT(!cmd.empty());

    struct Tmp {
        int error_code = 0;
        Callback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
        [tmp, cmd] {
            if (!util::ExecuteCmd(cmd))
                tmp->error_code = 1;
        },
        [tmp] {
            if (tmp->cb)
                tmp->cb(tmp->error_code);
        }
    );
}

void Async::executeCmd(const std::string &cmd, StringCallback &&cb)
{
    TBOX_ASSERT(!cmd.empty());
    TBOX_ASSERT(cb != nullptr);

    struct Tmp {
        int error_code = 0;
        std::string result;
        StringCallback cb;
    };

    auto tmp = std::make_shared<Tmp>();
    tmp->cb = std::move(cb);

    thread_pool_->execute(
        [tmp, cmd] {
            if (!util::ExecuteCmd(cmd, tmp->result))
                tmp->error_code = 1;
        },
        [tmp] {
            tmp->cb(tmp->error_code, tmp->result);
        }
    );
}

}
}
