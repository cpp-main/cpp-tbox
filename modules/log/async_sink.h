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
#ifndef TBOX_LOG_ASYNC_SINK_H_20220408
#define TBOX_LOG_ASYNC_SINK_H_20220408

#include "sink.h"

#include <vector>

#include <tbox/util/async_pipe.h>
#include <tbox/util/buffer.h>

namespace tbox {
namespace log {

class AsyncSink : public Sink {
  public:
    using Config = util::AsyncPipe::Config;

    void setConfig(const Config &cfg) { cfg_ = cfg; }
    void cleanup();

  protected:
    virtual void onEnable() override;
    virtual void onDisable() override;

    virtual void onLogFrontEnd(const LogContent *content) override;
    void onLogBackEndReadPipe(const void *data_ptr, size_t data_size);
    void onLogBackEnd(const LogContent &content);

    void append(const char *str, size_t len);
    void append(char ch);

    virtual void endline() = 0;
    virtual void flush() = 0;

  protected:
    std::vector<char> cache_;

  private:
    Config cfg_;
    util::AsyncPipe async_pipe_;
    bool is_pipe_inited_ = false;

    util::Buffer buffer_;
};

}
}

#endif //TBOX_LOG_ASYNC_SINK_H_20220408
