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
#ifndef TBOX_MAIN_LOG_H_20220414
#define TBOX_MAIN_LOG_H_20220414

#include <map>

#include <tbox/base/json_fwd.h>

#include <tbox/log/sync_stdout_sink.h>
#include <tbox/log/async_syslog_sink.h>
#include <tbox/log/async_file_sink.h>

#include <tbox/terminal/terminal_nodes.h>

#include "context.h"

namespace tbox {
namespace main {

class Log {
  public:
    void fillDefaultConfig(Json &cfg) const;
    bool initialize(const char *proc_name, Context &ctx, const Json &cfg);
    void cleanup();

  protected:
    struct SinkShellNodes {
        terminal::NodeToken dir;
        terminal::NodeToken set_enable;
        terminal::NodeToken set_color_enable;
        terminal::NodeToken set_level;
        terminal::NodeToken unset_level;
    };

    struct FileSinkShellNodes : public SinkShellNodes {
        terminal::NodeToken set_path;
        terminal::NodeToken set_prefix;
        terminal::NodeToken set_sync_enable;
        terminal::NodeToken set_max_size;
    };

    struct StdoutSink {
        log::SyncStdoutSink sink;
        SinkShellNodes nodes;
    };

    struct SyslogSink {
        log::AsyncSyslogSink sink;
        SinkShellNodes nodes;
    };

    struct FileSink {
        log::AsyncFileSink sink;
        FileSinkShellNodes nodes;
    };

  protected:
    //stdout相关
    bool installStdoutSink();
    bool initStdoutSinkByJson(const Json &js);
    bool uninstallStdoutSink();

    //syslog相关
    bool installSyslogSink();
    bool initSyslogSinkByJson(const Json &js);
    bool uninstallSyslogSink();

    //file相关
    bool installFileSink(const std::string &name);
    bool initFileSinkByJson(const std::string &name, const Json &js, const char *proc_name);
    bool uninstallFileSink(const std::string &name);
    bool uninstallFileSink(FileSink *file_sink, const std::string &name);

    //! 通过JSON初始化Sink共有的配置项
    void initSinkByJson(log::Sink &sink, const Json &js);

    void initShell();

    void installShellForSink(log::Sink &sink, terminal::NodeToken parent_node, SinkShellNodes &nodes, const std::string &name);
    void uninstallShellForSink(terminal::NodeToken parent_node, SinkShellNodes &nodes, const std::string &name);

    void installShellForFileSink(log::AsyncFileSink &sink, FileSinkShellNodes &nodes, const std::string &name);
    void uninstallShellForFileSink(FileSinkShellNodes &nodes, const std::string &name);

  private:
    terminal::TerminalNodes *shell_;

    StdoutSink *stdout_sink_ = nullptr;
    SyslogSink *syslog_sink_ = nullptr;
    std::map<std::string, FileSink*> file_sinks_;

    terminal::NodeToken sink_node_;
    terminal::NodeToken file_sink_node_;
};

}
}

#endif //TBOX_MAIN_LOG_H_20220414
