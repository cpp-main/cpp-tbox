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
#include <syslog.h>
#include "async_syslog_sink.h"

namespace tbox {
namespace log {

AsyncSyslogSink::AsyncSyslogSink()
{
    AsyncSink::Config cfg;
    cfg.buff_size = 10240;
    cfg.buff_min_num = 2;
    cfg.buff_max_num = 20;
    cfg.interval  = 100;

    setConfig(cfg);
}

void AsyncSyslogSink::endline()
{
    cache_.push_back('\0');
    ::syslog(LOG_INFO, "%s", cache_.data());

    cache_.clear();
}

}
}
