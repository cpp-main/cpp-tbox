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
  },
  "file": {
    "enable": false,
    "enable_color": false,
    "levels": {"":7},
    "max_size": 1024
  },
  "syslog": {
    "enable": false,
    "levels": {"":7}
  }
}
)"_json;
}

bool Log::initialize(const char *proc_name, Context &ctx, const Json &cfg)
{
    initShell(*ctx.terminal());

    if (util::json::HasObjectField(cfg, "log")) {
        auto &js_log = cfg.at("log");
        //! STDOUT
        if (util::json::HasObjectField(js_log, "stdout")) {
            auto &js_stdout = js_log.at("stdout");
            initSink(js_stdout, sync_stdout_sink_);
        }

        //! SYSLOG
        if (util::json::HasObjectField(js_log, "syslog")) {
            auto &js_syslog = js_log.at("syslog");
            initSink(js_syslog, async_syslog_sink_);
        }

        //! FILELOG
        if (util::json::HasObjectField(js_log, "file")) {
            auto &js_file = js_log.at("file");

            std::string path;
            if (util::json::GetField(js_file, "path", path))
                async_file_sink_.setFilePath(path);

            std::string prefix = util::fs::Basename(proc_name);
            util::json::GetField(js_file, "prefix", prefix);
            async_file_sink_.setFilePrefix(prefix);

            bool enable_sync = false;
            if (util::json::GetField(js_file, "enable_sync", enable_sync))
                async_file_sink_.setFileSyncEnable(enable_sync);

            unsigned int max_size = 0;
            if (util::json::GetField(js_file, "max_size", max_size))
                async_file_sink_.setFileMaxSize(max_size * 1024);

            initSink(js_file, async_file_sink_);
        }
    }
    return true;
}

void Log::cleanup()
{
    async_file_sink_.disable();
    async_syslog_sink_.disable();
    sync_stdout_sink_.disable();

    async_file_sink_.cleanup();
    async_syslog_sink_.cleanup();
}

void Log::initSink(const Json &js, log::Sink &ch)
{
    bool enable = false;
    if (util::json::GetField(js, "enable", enable)) {
        if (enable)
            ch.enable();
        else
            ch.disable();
    }

    bool enable_color = false;
    if (util::json::GetField(js, "enable_color", enable_color))
        ch.enableColor(enable_color);

    if (util::json::HasObjectField(js, "levels")) {
        auto &js_levels = js.at("levels");
        for (auto &item : js_levels.items())
            ch.setLevel(item.key(), item.value());
    }
}

void Log::initShell(TerminalNodes &term)
{
    auto log_node = term.createDirNode("This is log directory");
    term.mountNode(term.rootNode(), log_node, "log");

    {
        auto dir_node = term.createDirNode();
        term.mountNode(log_node, dir_node, "stdout");
        initShellForSink(sync_stdout_sink_, term, dir_node);
    }
    {
        auto dir_node = term.createDirNode();
        term.mountNode(log_node, dir_node, "syslog");
        initShellForSink(async_syslog_sink_, term, dir_node);
    }
    {
        auto dir_node = term.createDirNode();
        term.mountNode(log_node, dir_node, "file");
        initShellForSink(async_file_sink_, term, dir_node);
        initShellForAsyncFileSink(term, dir_node);
    }
}

void Log::initShellForSink(log::Sink &log_ch, terminal::TerminalNodes &term, terminal::NodeToken dir_node)
{
    {
        terminal::BooleanFuncNodeProfile profile;
        profile.set_func = \
            [&log_ch] (bool is_enable) {
                if (is_enable)
                    log_ch.enable();
                else
                    log_ch.disable();
                return true;
            };
        profile.usage = "Usage: set_enable on|off\r\n";
        profile.help = "Enable or disable log sink";
        terminal::AddFuncNode(term, dir_node, "set_enable", profile);
    }

    {
        terminal::BooleanFuncNodeProfile profile;
        profile.set_func = \
            [&log_ch] (bool is_enable) {
                log_ch.enableColor(is_enable);
                return true;
            };
        profile.usage = "Usage: set_color_enable on|off\r\n";
        profile.help = "Enable or disable log color";
        terminal::AddFuncNode(term, dir_node, "set_color_enable", profile);
    }

    {
        auto func_node = term.createFuncNode(
            [&log_ch] (const Session &s, const Args &args) {
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

                        log_ch.setLevel(module_id, level);
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
        term.mountNode(dir_node, func_node, "set_level");
    }

    {
        terminal::StringFuncNodeProfile profile;
        profile.set_func = \
            [&log_ch] (const std::string &module) {
                log_ch.unsetLevel(module);
                return true;
            };
        profile.usage = "Usage: unset_level <module>\r\n";
        profile.help = "Unset log level";
        terminal::AddFuncNode(term, dir_node, "unset_level", profile);
    }
}

void Log::initShellForAsyncFileSink(terminal::TerminalNodes &term, terminal::NodeToken dir_node)
{
    {
        terminal::StringFuncNodeProfile profile;
        profile.set_func = \
            [this] (const std::string &path) {
                async_file_sink_.setFilePath(path);
                return true;
            };
        profile.usage = \
            "Usage: set_path <path>\r\n"
            "Exp  : set_path /var/log/\r\n";
        profile.help = "Set log file path";
        terminal::AddFuncNode(term, dir_node, "set_path", profile);
    }

    {
        terminal::StringFuncNodeProfile profile;
        profile.set_func = \
            [this] (const std::string &prefix) {
                async_file_sink_.setFilePrefix(prefix);
                return true;
            };
        profile.usage = "Usage: set_prefix <prefix>\r\n";
        profile.help = "Set log file prefix";
        terminal::AddFuncNode(term, dir_node, "set_prefix", profile);
    }

    {
        terminal::BooleanFuncNodeProfile profile;
        profile.set_func = \
            [this] (bool is_enable) {
                async_file_sink_.setFileSyncEnable(is_enable);
                return true;
            };
        profile.usage = "Usage: set_sync_enable on|off\r\n";
        profile.help = "Enable or disable file sync";
        terminal::AddFuncNode(term, dir_node, "set_sync_enable", profile);
    }

    {
        terminal::IntegerFuncNodeProfile profile;
        profile.set_func = \
            [this] (int max_size) {
                async_file_sink_.setFileMaxSize(max_size * 1024);
                return true;
            };
        profile.min_value = 1;
        profile.usage = \
            "Usage: set_max_size <size>  # Unit KB, >=1\r\n"
            "Exp  : set_max_size 1024\r\n";
        profile.help = "Set log file max size";
        terminal::AddFuncNode(term, dir_node, "set_max_size", profile);
    }
}

}
}

