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
#include "terminal.h"

#include <iomanip>
#include <sstream>

#include <tbox/base/log.h>
#include <tbox/util/split_cmdline.h>
#include <tbox/util/string.h>
#include <tbox/event/loop.h>

#include "session_context.h"
#include "dir_node.h"
#include "func_node.h"
#include "../connection.h"

namespace tbox {
namespace terminal {

using namespace std;

bool Terminal::Impl::execute(SessionContext *s)
{
    std::vector<std::string> cmdlines;
    util::string::Split(s->curr_input, ";", cmdlines);

    for (auto &cmdline : cmdlines) {
        if (!executeCmd(s, cmdline))
            return false;
    }

    return true;
}

bool Terminal::Impl::executeCmd(SessionContext *s, const std::string &cmdline)
{
    if (cmdline.empty())
        return false;

    LogInfo("cmdline: %s", cmdline.c_str());

    vector<string> args;
    if (!util::SplitCmdline(cmdline, args) || args.empty()) {
        s->wp_conn->send(s->token, "Error: parse cmdline fail!\r\n");
        return true;
    }

    const auto &cmd = args[0];
    if (cmd == "ls") {
        executeLsCmd(s, args);
    } else if (cmd == "pwd") {
        executePwdCmd(s, args);
    } else if (cmd == "cd") {
        executeCdCmd(s, args);
    } else if (cmd == "help") {
        executeHelpCmd(s, args);
    } else if (cmd == "history") {
        executeHistoryCmd(s, args);
        return false;   //! 查看历史命令不要再存历史
    } else if (cmd == "exit" || cmd == "quit") {
        executeExitCmd(s, args);
    } else if (cmd == "tree") {
        executeTreeCmd(s, args);
    } else if (cmd[0] == '!') {
        return executeRunHistoryCmd(s, args);
    } else {
        executeUserCmd(s, args);
    }
    return true;
}

void Terminal::Impl::executeCdCmd(SessionContext *s, const Args &args)
{
    string path_str = "/";
    if (args.size() >= 2)
        path_str = args[1];

    stringstream ss;

    auto node_path = s->path;
    bool is_found = findNode(path_str, node_path);
    if (is_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        if (top_node == nullptr) {
            ss << "Error: '" << path_str << "' node has been deleted." << "\r\n";
        } else if (top_node->type() == NodeType::kDir) {
            s->path = node_path;
        } else {
            ss << "Error: '" << path_str << "' not directory." << "\r\n";
        }
    } else {
        ss << "Error: cannot access '" << path_str << "'.\r\n";
    }

    s->wp_conn->send(s->token, ss.str());
}

void Terminal::Impl::executeHelpCmd(SessionContext *s, const Args &args)
{
    if (args.size() < 2) {
        printHelp(s);
        return;
    }

    stringstream ss;

    string path_str = args[1];
    auto node_path = s->path;
    bool is_found = findNode(path_str, node_path);
    if (is_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        if (top_node != nullptr) {
            ss << top_node->help() << "\r\n";
        } else {
            ss << "Error: '" << path_str << "' node has been deleted.\r\n";
        }
    } else {
        ss << "Error: cannot access '" << path_str << "'.\r\n";
    }

    s->wp_conn->send(s->token, ss.str());
}

void Terminal::Impl::executeLsCmd(SessionContext *s, const Args &args)
{
    string path_str = ".";
    if (args.size() >= 2)
        path_str = args[1];

    stringstream ss;

    auto node_path = s->path;
    bool is_found = findNode(path_str, node_path);
    if (is_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        if (top_node == nullptr) {
            ss << "Error: '" << path_str << "' node has been deleted.\r\n";
        } else if (top_node->type() == NodeType::kDir) {
            auto top_dir_node = static_cast<DirNode*>(top_node);
            vector<NodeInfo> node_info_vec;
            top_dir_node->children(node_info_vec);

            for (auto item : node_info_vec) {
                ss << "- " << item.name;
                auto node = nodes_.at(item.token);
                if (node == nullptr) {
                    ss << "(X)";
                } else if (node->type() == NodeType::kDir)
                    ss << '/';
                ss << "\r\n";
            }
            ss << "\r\n";
        } else {
            ss << path_str << " is function" << ".\r\n";
        }
    } else {
        ss << "Error: cannot access '" << path_str << "'.\r\n";
    }

    s->wp_conn->send(s->token, ss.str());
}

void Terminal::Impl::executeHistoryCmd(SessionContext *s, const Args &)
{
    stringstream ss;
    for (size_t i = 0; i < s->history.size(); ++i) {
        const auto &cmd = s->history.at(i);
        ss << setw(2) << i << "  " << cmd << "\r\n";
    }
    s->wp_conn->send(s->token, ss.str());
}

void Terminal::Impl::executeExitCmd(SessionContext *s, const Args &)
{
    if (!(s->options & kQuietMode))
        s->wp_conn->send(s->token, "Bye!\r\n");

    wp_loop_->runNext(
        [this, s] {
            s->wp_conn->endSession(s->token);
            deleteSession(s->token);
        },
        __func__
    );
}

void Terminal::Impl::executeTreeCmd(SessionContext *s, const Args &args)
{
    string path_str = ".";
    if (args.size() >= 2)
        path_str = args[1];

    stringstream ss;

    auto node_path = s->path;
    bool is_found = findNode(path_str, node_path);

    if (is_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        if (top_node == nullptr) {
            ss << node_path.back().first << " node has been deleted.\r\n";
        } else if (top_node->type() == NodeType::kDir) {
            vector<vector<NodeInfo>> node_token_stack;    //!< 遍历栈
            string indent_str;  //!< 缩进字串

            {
                //! 先将被 tree 结点的子节点找出来，压入到 node_token_stack 中
                auto top_dir_node = static_cast<DirNode*>(top_node);
                vector<NodeInfo> node_info_vec;
                top_dir_node->children(node_info_vec);
                if (!node_info_vec.empty())
                    node_token_stack.push_back(node_info_vec);
            }
/**
 * Print like below:
 * |-- a
 * |   |-- aa
 * |   |   |-- aaa
 * |   |   `-- aab
 * |   `-- ab
 * |-- b
 * |   |-- ba
 * |   `-- bb
 * `-- c
 *     `- ca
 */
            //! 开始循环遍历，深度搜索
            while (!node_token_stack.empty()) {
                auto &last_level = node_token_stack.back();

                while (!last_level.empty()) {
                    //! 取出当前层级的第一个Node进行处理
                    bool is_last_node = last_level.size() == 1;
                    const char *curr_indent_str = is_last_node ? "`-- " : "|-- ";

                    auto &curr_node_info = last_level.front();
                    ss << indent_str << curr_indent_str << curr_node_info.name;

                    auto curr_node = nodes_.at(curr_node_info.token);
                    if (curr_node == nullptr) {
                        //! 如果已被删除了的结点，显示(X)
                        ss << "(X)\r\n";
                    } else if (curr_node->type() == NodeType::kFunc) {
                        //! 如果是Func，就打印一下名称就可以了
                        ss << "\r\n";
                    } else if (curr_node->type() == NodeType::kDir) {
                        //! 如果是Dir，则需要再深入地遍历其子Node
                        //! 首先需要查重，防止循环路径引起的死循环
                        bool is_repeat = curr_node_info.token == root_token_;
                        for (size_t i = 0; !is_repeat && i < node_token_stack.size() - 1; ++i) {
                            if (node_token_stack[i].front().token == curr_node_info.token)
                                is_repeat = true;
                        }

                        if (is_repeat)
                            ss << "(R)";

                        ss << "\r\n";

                        if (!is_repeat) {
                            //! 找出该Dir下所有的子Node。
                            auto curr_dir_node = static_cast<DirNode*>(curr_node);
                            vector<NodeInfo> node_info_vec;
                            curr_dir_node->children(node_info_vec);
                            if (!node_info_vec.empty()) {
                                //! 如果存在子Node，则将Node列表压入到node_token_stack，添加新的层级
                                node_token_stack.push_back(node_info_vec);
                                indent_str += (is_last_node ? "    " : "|   "); //! 同时修改缩时字串
                                break;
                            }
                        }
                    }

                    //! 运行到这里，表示已完成一个Node的处理。开始做清理操作
                    while (!node_token_stack.empty()) {
                        node_token_stack.back().erase(node_token_stack.back().begin()); //! 删除当前Node
                        if (node_token_stack.back().empty()) {  //! 检查当前级是否已经没有Node需要遍历了
                            node_token_stack.pop_back();    //! 如果是，则弹出该层级
                            if (!indent_str.empty())
                                indent_str.erase(indent_str.size() - 4); //! 同步修改缩进字串
                        } else
                            break;
                    }
                }
            }
        } else {
            ss << node_path.back().first << " is a function.\r\n";
        }
    } else {
        ss << "Error: cannot access '" << path_str << "'.\r\n";
    }

    s->wp_conn->send(s->token, ss.str());
}

void Terminal::Impl::executePwdCmd(SessionContext *s, const Args &)
{
    stringstream ss;
    ss << '/';
    for (size_t i = 0; i < s->path.size(); ++i) {
        ss << s->path.at(i).first;
        if ((i + 1) != s->path.size())
            ss << '/';
    }
    ss << "\r\n";
    s->wp_conn->send(s->token, ss.str());
}

bool Terminal::Impl::executeRunHistoryCmd(SessionContext *s, const Args &args)
{
    string sub_cmd = args[0].substr(1);
    if (sub_cmd == "!") {
        s->curr_input = s->history.back();
        return execute(s);
    }

    try {
        auto index = std::stoi(sub_cmd);
        bool is_index_valid = false;
        if (index >= 0) {
            if (static_cast<size_t>(index) < s->history.size()) {
                s->curr_input = s->history.at(index);
                is_index_valid = true;
            }
        } else {
            if (s->history.size() >= static_cast<size_t>(-index)) {
                s->curr_input = s->history.at(s->history.size() + index);
                is_index_valid = true;
            }
        }

        if (is_index_valid) {
            s->wp_conn->send(s->token, s->curr_input + "\r\n");
            return execute(s);
        } else
            s->wp_conn->send(s->token, "Error: index out of range.\r\n");
    } catch (const invalid_argument &e) {
        s->wp_conn->send(s->token, "Error: parse index fail.\r\n");
    }

    return false;
}

void Terminal::Impl::executeUserCmd(SessionContext *s, const Args &args)
{
    stringstream ss;

    const auto &cmd = args[0];
    auto node_path = s->path;
    bool is_cmd_found = findNode(cmd, node_path);

    if (is_cmd_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        if (top_node == nullptr) {
            ss << "Error: '" << cmd << "' node has been deleted.\r\n";
        } else if (top_node->type() == NodeType::kFunc) {
            auto top_func_node = static_cast<FuncNode*>(top_node);
            Session session(s->wp_conn, s->token);
            top_func_node->execute(session, args);
        } else {
            s->path = node_path;
        }
    } else {
        ss << "Error: '" << cmd << "' not found.\r\n";
    }

    s->wp_conn->send(s->token, ss.str());
}

}
}
