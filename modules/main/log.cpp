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
    "levels": {"":6}
  },
  "file": {
    "enable": false,
    "enable_color": false,
    "levels": {"":4},
    "max_size": 1024
  },
  "syslog": {
    "enable": false,
    "levels": {"":4}
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
        auto func_node = term.createFuncNode(
            [&log_ch] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    if (opt == "on") {
                        log_ch.enable();
                        oss << "on\r\n";
                        print_usage = false;
                    } else if (opt == "off") {
                        log_ch.disable();
                        oss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    oss << "Usage: " << args[0] << " on|off\r\n";

                s.send(oss.str());
            }
        , "enable or disable");
        term.mountNode(dir_node, func_node, "enable");
    }

    {
        auto func_node = term.createFuncNode(
            [&log_ch] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    if (opt == "on") {
                        log_ch.enableColor(true);
                        oss << "on\r\n";
                        print_usage = false;
                    } else if (opt == "off") {
                        log_ch.enableColor(false);
                        oss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    oss << "Usage: " << args[0] << " on|off\r\n";

                s.send(oss.str());
            }
        , "enable or disable color");
        term.mountNode(dir_node, func_node, "enable_color");
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
                    oss << "Usage: " << args[0] << " <module_id:string> <level:0-6>\r\n"
                       << "LEVEL\r\n"
                       << " 0: Fatal\r\n"
                       << " 1: Error\r\n"
                       << " 2: Warn\r\n"
                       << " 3: Notice\r\n"
                       << " 4: Info\r\n"
                       << " 5: Debug\r\n"
                       << " 6: Trace\r\n"
                       ;

                s.send(oss.str());
            }
        , "set log level");
        term.mountNode(dir_node, func_node, "set_level");
    }

    {
        auto func_node = term.createFuncNode(
            [&log_ch] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    log_ch.unsetLevel(args.at(1));
                    print_usage = false;
                    oss << "done\r\n";
                }

                if (print_usage)
                    oss << "Usage: " << args[0] << " <module>\r\n";

                s.send(oss.str());
            }
        , "unset log level");
        term.mountNode(dir_node, func_node, "unset_level");
    }
}

void Log::initShellForAsyncFileSink(terminal::TerminalNodes &term, terminal::NodeToken dir_node)
{
    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    async_file_sink_.setFilePath(args.at(1));
                    print_usage = false;
                    oss << "done\r\n";
                }

                if (print_usage) {
                    oss << "Usage: " << args[0] << " <path>\r\n"
                        << "Exp  : " << args[0] << " /var/log/\r\n";
                }

                s.send(oss.str());
            }
        , "set log file path");
        term.mountNode(dir_node, func_node, "set_path");
    }

    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    async_file_sink_.setFilePrefix(args.at(1));
                    print_usage = false;
                    oss << "done\r\n";
                }

                if (print_usage)
                    oss << "Usage: " << args[0] << " <prefix>\r\n";

                s.send(oss.str());
            }
        , "set log file prefix");
        term.mountNode(dir_node, func_node, "set_prefix");
    }

    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    const auto &opt = args[1];
                    if (opt == "on") {
                        async_file_sink_.setFileSyncEnable(true);
                        oss << "on\r\n";
                        print_usage = false;
                    } else if (opt == "off") {
                        async_file_sink_.setFileSyncEnable(false);
                        oss << "off\r\n";
                        print_usage = false;
                    }
                }

                if (print_usage)
                    oss << "Usage: " << args[0] << " on|off\r\n";

                s.send(oss.str());
            }
        , "enable or disable file sync");
        term.mountNode(dir_node, func_node, "enable_sync");
    }

    {
        auto func_node = term.createFuncNode(
            [this] (const Session &s, const Args &args) {
                std::ostringstream oss;
                bool print_usage = true;
                if (args.size() >= 2) {
                    size_t max_size;
                    bool is_throw = CatchThrowQuietly([&] { max_size = std::stoul(args.at(1)); });
                    if (!is_throw) {
                        async_file_sink_.setFileMaxSize(max_size * 1024);
                        print_usage = false;
                        oss << "done, max_size: " << max_size << " KB\r\n";
                    } else {
                        oss << "size must be number\r\n";
                    }
                }

                if (print_usage) {
                    oss << "Usage: " << args[0] << " <size>\r\n"
                        << "Exp  : " << args[0] << " 1024\r\n";
                }

                s.send(oss.str());
            }
        , "set log file max size");
        term.mountNode(dir_node, func_node, "set_max_size");
    }

}

}
}

