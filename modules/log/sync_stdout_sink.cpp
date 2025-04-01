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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "sync_stdout_sink.h"

#include <unistd.h>
#include <algorithm>

namespace tbox {
namespace log {

void SyncStdoutSink::onLogFrontEnd(const LogContent *content)
{
    updateTimestampStr(content->timestamp.sec);

    //! 开启色彩，显示日志等级
    if (enable_color_)
        printf("\033[%sm", LOG_LEVEL_COLOR_CODE[content->level]);

    //! 打印等级、时间戳、线程号、模块名
    printf("%c %s.%06u %ld %s ", LOG_LEVEL_LEVEL_CODE[content->level],
           timestamp_str_, content->timestamp.usec,
           content->thread_id, content->module_id);

    if (content->func_name != nullptr)
        printf("%s() ", content->func_name);

    if (content->text_len > 0)
        printf("%.*s ", content->text_len, content->text_ptr);

    if (content->text_trunc)
        printf("(TRUNCATED) ");

    if (content->file_name != nullptr)
        printf("-- %s:%d", content->file_name, content->line);

    if (enable_color_)
        puts("\033[0m");    //! 恢复色彩
    else
        putchar('\n');
}

}
}
