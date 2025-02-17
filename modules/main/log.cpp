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
#include "log.h"
#include <iostream>
#include <sstream>
#include <tbox/base/json.hpp>
#include <tbox/base/catch_throw.h>
#include <tbox/terminal/session.h>
#include <tbox/terminal/helper.h>
#include <tbox/util/fs.h>
#include <tbox/util/json.h>

namespace tbox {
namespace main {

using namespace terminal;
using namespace std;

void Log::fillDefaultConfig(Json &cfg) const
{
    cfg["log"] = R"(
 {
  "stdout": {
    "enable": true,
    "enable_color": true,
    "levels": {"":7}
  }
}
)"_json;
}

bool Log::initialize(const char *proc_name, Context &ctx, const Json &cfg)
{
    shell_ = ctx.terminal();
    initShell();

    if (util::json::HasObjectField(cfg, "log")) {
        auto &js_log = cfg.at("log");

        int max_len = 0;
        if (util::json::GetField(js_log, "max_len", max_len) && max_len > 0) {
            LogSetMaxLength(static_cast<size_t>(max_len));
        }

        //! STDOUT
        if (util::json::HasObjectField(js_log, "stdout")) {
            installStdoutSink();
            initStdoutSinkByJson(js_log.at("stdout"));
        }
        //! SYSLOG
        if (util::json::HasObjectField(js_log, "syslog")) {
            installSyslogSink();
            initSyslogSinkByJson(js_log.at("syslog"));
        }

#if 0
        //! FILELOG
        if (util::json::HasObjectField(js_log, "files")) {
            auto &js_files = js_log.at("files");
            for (auto &js_file : js_files.items()) {
                auto async_file_sink = new log::AsyncFileSink;
                async_file_sink_map_[js_file.key()] = async_file_sink;
                //TODO
            }
        }
#endif
    }
    return true;
}

void Log::initShell()
{
    auto log_node = terminal::AddDirNode(*shell_, shell_->rootNode(), "log", "This is log directory");
    sink_node_ = terminal::AddDirNode(*shell_, log_node, "sinks");

    {
        terminal::IntegerFuncNodeProfile profile;
        profile.set_func = \
            [] (int max_len) {
                if (max_len > 0) {
                    LogSetMaxLength(static_cast<size_t>(max_len));
                    return true;
                }
                return false;
            };
        profile.get_func = [] { return LogGetMaxLength(); };
        profile.usage = "Usage: max_len       # get max len, unit:byte\r\n"
                        "       max_len <len> # set max len, len>0\r\n";
        profile.help = "get or set log maxmum length";
        terminal::AddFuncNode(*shell_, log_node, "max_len", profile);
    }

    terminal::AddFuncNode(*shell_, log_node, "add_stdout", [this] { installStdoutSink(); });
    terminal::AddFuncNode(*shell_, log_node, "add_syslog", [this] { installSyslogSink(); });
    terminal::AddFuncNode(*shell_, log_node, "del_stdout", [this] { uninstallStdoutSink(); });
    terminal::AddFuncNode(*shell_, log_node, "del_syslog", [this] { uninstallSyslogSink(); });

    {
        terminal::StringFuncNodeProfile profile;
        profile.set_func = [this] (const std::string &name) {
            return installFileSink(name);
        };
        terminal::AddFuncNode(*shell_, log_node, "add_file", profile);
    }

    {
        terminal::StringFuncNodeProfile profile;
        profile.set_func = [this] (const std::string &name) {
            return uninstallFileSink(name);
        };
        terminal::AddFuncNode(*shell_, log_node, "del_file", profile);
    }
}

bool Log::installStdoutSink()
{
    if (stdout_sink_ != nullptr)
        return false;

    stdout_sink_ = new StdoutSink;
    installShellForSink(stdout_sink_->sink, stdout_sink_->nodes, "stdout");

    return true;
}

bool Log::initStdoutSinkByJson(const Json &js)
{
    initSinkByJson(stdout_sink_->sink, js);
    return true;
}

bool Log::uninstallStdoutSink()
{
    if (stdout_sink_ == nullptr)
        return false;

    uninstallShellForSink(stdout_sink_->nodes, "stdout");

    stdout_sink_->sink.disable();
    CHECK_DELETE_RESET_OBJ(stdout_sink_);
    return true;
}

bool Log::installSyslogSink()
{
    if (syslog_sink_ != nullptr)
        return false;

    syslog_sink_ = new SyslogSink;
    installShellForSink(syslog_sink_->sink, syslog_sink_->nodes, "syslog");

    return true;
}

bool Log::initSyslogSinkByJson(const Json &js)
{
    initSinkByJson(syslog_sink_->sink, js);
    return true;
}

bool Log::uninstallSyslogSink()
{
    if (syslog_sink_ == nullptr)
        return false;

    uninstallShellForSink(syslog_sink_->nodes, "syslog");

    syslog_sink_->sink.disable();
    syslog_sink_->sink.cleanup();
    CHECK_DELETE_RESET_OBJ(syslog_sink_);
    return true;
}

bool Log::installFileSink(const std::string &name)
{
    //! 不能创建已有的通道名
    if (file_sinks_.find(name) != file_sinks_.end())
        return false;

    auto file_sink = new FileSink;
    installShellForFileSink(file_sink->sink, file_sink->nodes, std::string("file:") + name);
    file_sinks_.emplace(name, file_sink);
    return true;
}

#if 0
bool Log::initFileSinkByJson(const std::string &name, const Json &js)
{
    std::string path;
    if (util::json::GetField(js_file, "path", path))
        async_file_sink->setFilePath(path);

    std::string prefix = util::fs::Basename(proc_name);
    util::json::GetField(js_file, "prefix", prefix);
    async_file_sink->setFilePrefix(prefix);

    bool enable_sync = false;
    if (util::json::GetField(js_file, "enable_sync", enable_sync))
        async_file_sink->setFileSyncEnable(enable_sync);

    unsigned int max_size = 0;
    if (util::json::GetField(js_file, "max_size", max_size))
        async_file_sink->setFileMaxSize(max_size * 1024);

    initSinkByJson(*async_file_sink, js_file);

    return true;
}
#endif

bool Log::uninstallFileSink(const std::string &name)
{
    auto iter = file_sinks_.find(name);
    if (iter == file_sinks_.end())
        return false;

    auto file_sink = iter->second;
    file_sinks_.erase(iter);

    uninstallShellForFileSink(file_sink->nodes, std::string("file:") + name);

    file_sink->sink.disable();
    file_sink->sink.cleanup();
    CHECK_DELETE_OBJ(file_sink);
    return true;
}

void Log::cleanup()
{
    uninstallStdoutSink();
    uninstallSyslogSink();

    //TODO
}

void Log::initSinkByJson(log::Sink &sink, const Json &js)
{
    bool enable = false;
    if (util::json::GetField(js, "enable", enable)) {
        if (enable)
            sink.enable();
        else
            sink.disable();
    }

    bool enable_color = false;
    if (util::json::GetField(js, "enable_color", enable_color))
        sink.enableColor(enable_color);

    if (util::json::HasObjectField(js, "levels")) {
        auto &js_levels = js.at("levels");
        for (auto &item : js_levels.items())
            sink.setLevel(item.key(), item.value());
    }
}

void Log::installShellForSink(log::Sink &sink, SinkShellNodes &nodes, const std::string &name)
{
    nodes.dir = terminal::AddDirNode(*shell_, sink_node_, name);

    {
        terminal::BooleanFuncNodeProfile profile;
        profile.set_func = \
            [&sink] (bool is_enable) {
                if (is_enable)
                    sink.enable();
                else
                    sink.disable();
                return true;
            };
        profile.usage = "Usage: set_enable on|off\r\n";
        profile.help = "Enable or disable log sink";
        nodes.set_enable = terminal::AddFuncNode(*shell_, nodes.dir, "set_enable", profile);
    }

    {
        terminal::BooleanFuncNodeProfile profile;
        profile.set_func = \
            [&sink] (bool is_enable) {
                sink.enableColor(is_enable);
                return true;
            };
        profile.usage = "Usage: set_color_enable on|off\r\n";
        profile.help = "Enable or disable log color";
        nodes.set_color_enable = terminal::AddFuncNode(*shell_, nodes.dir, "set_color_enable", profile);
    }

    {
        auto func_node = shell_->createFuncNode(
            [&sink] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 3) {
                    do {
                        auto &module_id = args[1];
                        int level = 0;
                        if (CatchThrowQuietly([&] { level = std::stoi(args[2]); })) {
                            oss << "level must be number\r\n";
                            break;
                        }
                        if (level < 0 || level > LOG_LEVEL_TRACE) {
                            oss << "level range: [0-" << LOG_LEVEL_TRACE << "]\r\n";
                            break;
                        }

                        sink.setLevel(module_id, level);
                        oss << "done. level: " << level << "\r\n";
                        print_usage = false;

                    } while (false);
                }

                if (print_usage)
                    oss << "Usage: " << args[0] << " <module> <level:0-7>\r\n"
                       << "LEVEL\r\n"
                       << " 0: Fatal\r\n"
                       << " 1: Error\r\n"
                       << " 2: Warn\r\n"
                       << " 3: Notice\r\n"
                       << " 4: Important\r\n"
                       << " 5: Info\r\n"
                       << " 6: Debug\r\n"
                       << " 7: Trace\r\n"
                       ;

                s.send(oss.str());
            }
        , "set log level");
        shell_->mountNode(nodes.dir, func_node, "set_level");
        nodes.set_level = func_node;
    }

    {
        terminal::StringFuncNodeProfile profile;
        profile.set_func = \
            [&sink] (const std::string &module) {
                sink.unsetLevel(module);
                return true;
            };
        profile.usage = "Usage: unset_level <module>\r\n";
        profile.help = "Unset log level";
        nodes.unset_level = terminal::AddFuncNode(*shell_, nodes.dir, "unset_level", profile);
    }
}

void Log::uninstallShellForSink(SinkShellNodes &nodes, const std::string &name)
{
    shell_->deleteNode(nodes.dir);
    shell_->deleteNode(nodes.set_enable);
    shell_->deleteNode(nodes.set_color_enable);
    shell_->deleteNode(nodes.set_level);
    shell_->deleteNode(nodes.unset_level);

    shell_->umountNode(sink_node_, name);
}

void Log::installShellForFileSink(log::AsyncFileSink &sink, FileSinkShellNodes &nodes, const std::string &name)
{
    installShellForSink(sink, nodes, name);

    {
        terminal::StringFuncNodeProfile profile;
        profile.set_func = \
            [this, &sink] (const std::string &path) {
                sink.setFilePath(path);
                return true;
            };
        profile.usage = \
            "Usage: set_path <path>\r\n"
            "Exp  : set_path /var/log/\r\n";
        profile.help = "Set log file path";
        nodes.set_path = terminal::AddFuncNode(*shell_, nodes.dir, "set_path", profile);
    }

    {
        terminal::StringFuncNodeProfile profile;
        profile.set_func = \
            [this, &sink] (const std::string &prefix) {
                sink.setFilePrefix(prefix);
                return true;
            };
        profile.usage = "Usage: set_prefix <prefix>\r\n";
        profile.help = "Set log file prefix";
        nodes.set_prefix = terminal::AddFuncNode(*shell_, nodes.dir, "set_prefix", profile);
    }

    {
        terminal::BooleanFuncNodeProfile profile;
        profile.set_func = \
            [this, &sink] (bool is_enable) {
                sink.setFileSyncEnable(is_enable);
                return true;
            };
        profile.usage = "Usage: set_sync_enable on|off\r\n";
        profile.help = "Enable or disable file sync";
        nodes.set_sync_enable = terminal::AddFuncNode(*shell_, nodes.dir, "set_sync_enable", profile);
    }

    {
        terminal::IntegerFuncNodeProfile profile;
        profile.set_func = \
            [this, &sink] (int max_size) {
                sink.setFileMaxSize(max_size * 1024);
                return true;
            };
        profile.min_value = 1;
        profile.usage = \
            "Usage: set_max_size <size>  # Unit KB, >=1\r\n"
            "Exp  : set_max_size 1024\r\n";
        profile.help = "Set log file max size";
        nodes.set_max_size = terminal::AddFuncNode(*shell_, nodes.dir, "set_max_size", profile);
    }
}

void Log::uninstallShellForFileSink(FileSinkShellNodes &nodes, const std::string &name)
{
    shell_->deleteNode(nodes.set_path);
    shell_->deleteNode(nodes.set_prefix);
    shell_->deleteNode(nodes.set_sync_enable);
    shell_->deleteNode(nodes.set_max_size);

    uninstallShellForSink(nodes, name);
}

}
}

