#include "terminal.h"

#include <sstream>

#include <tbox/base/log.h>
#include <tbox/util/split_cmdline.h>

#include "session_imp.h"
#include "dir_node.h"
#include "func_node.h"

namespace tbox::terminal {

using namespace std;

void Terminal::Impl::executeCmdline(SessionImpl *s, bool &store_in_history, bool &recover_cmdline)
{
    auto cmdline = s->curr_input;
    if (cmdline.empty())
        return;

    LogTrace("cmdline: %s", cmdline.c_str());

    vector<string> args;
    if (!util::SplitCmdline(cmdline, args) || args.empty()) {
        s->send("Error: parse cmdline fail!\r\n");
        return;
    }

    store_in_history = true;
    recover_cmdline = false;

    const auto &cmd = args[0];
    if (cmd == "ls") {
        bool is_succ = executeLsCmd(s, args);
        store_in_history = is_succ;
        recover_cmdline = !is_succ;
    } else if (cmd == "pwd") {
        executePwdCmd(s, args);
    } else if (cmd == "cd") {
        bool is_succ = executeCdCmd(s, args);
        store_in_history = is_succ;
        recover_cmdline = !is_succ;
    } else if (cmd == "help") {
        bool is_succ = executeHelpCmd(s, args);
        store_in_history = is_succ;
        recover_cmdline = !is_succ;
    } else if (cmd == "history") {
        executeHistoryCmd(s, args);
    } else if (cmd == "exit") {
        executeExitCmd(s, args);
    } else if (cmd == "tree") {
        bool is_succ = executeTreeCmd(s, args);
        store_in_history = is_succ;
        recover_cmdline = !is_succ;
    } else {
        bool is_succ = executeUserCmd(s, args);
        store_in_history = is_succ;
        recover_cmdline = !is_succ;
    }
}

bool Terminal::Impl::executeCdCmd(SessionImpl *s, const Args &args)
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
        if (top_node->type() == NodeType::kDir) {
            s->path = node_path;
            return true;
        } else {
            ss << "Error: '" << path_str << "' not directory" << "\r\n";
        }
    } else {
        ss << "Error: cannot access '" << path_str << "'\r\n";
    }

    s->send(ss.str());
    return false;
}

bool Terminal::Impl::executeHelpCmd(SessionImpl *s, const Args &args)
{
    if (args.size() < 2) {
        printHelp(s);
        return true;
    }

    stringstream ss;
    bool is_succ = false;

    string path_str = args[1];
    auto node_path = s->path;
    bool is_found = findNode(path_str, node_path);
    if (is_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        ss << top_node->help() << "\r\n";
        is_succ = true;
    } else {
        ss << "Error: cannot access '" << path_str << "'\r\n";
    }

    s->send(ss.str());
    return is_succ;
}

bool Terminal::Impl::executeLsCmd(SessionImpl *s, const Args &args)
{
    string path_str = ".";
    if (args.size() >= 2)
        path_str = args[1];

    stringstream ss;
    bool is_succ = false;

    auto node_path = s->path;
    bool is_found = findNode(path_str, node_path);
    if (is_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        if (top_node->type() == NodeType::kDir) {
            auto top_dir_node = static_cast<DirNode*>(top_node);
            vector<NodeInfo> node_info_vec;
            top_dir_node->children(node_info_vec);

            for (auto item : node_info_vec)
                ss << item.name << "\r\n";
            is_succ = true;
        } else {
            ss << path_str << " is function" << "\r\n";
        }
    } else {
        ss << "Error: cannot access '" << path_str << "'\r\n";
    }

    s->send(ss.str());
    return is_succ;
}

void Terminal::Impl::executeHistoryCmd(SessionImpl *s, const Args &args)
{
    stringstream ss;
    for (const auto &cmd : s->history)
        ss << cmd << "\r\n";
    s->send(ss.str());
}

void Terminal::Impl::executeExitCmd(SessionImpl *s, const Args &args)
{
    s->send("Bye!\r\n");
    s->endSession();
}

bool Terminal::Impl::executeTreeCmd(SessionImpl *s, const Args &args)
{
    string path_str = ".";
    if (args.size() >= 2)
        path_str = args[1];

    stringstream ss;
    bool is_succ = false;

    auto node_path = s->path;
    bool is_found = findNode(path_str, node_path);

    if (is_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        if (top_node->type() == NodeType::kDir) {
            vector<vector<NodeInfo>> node_token_stack;    //! 遍历记录
            string indent_str;

            {
                auto top_dir_node = static_cast<DirNode*>(top_node);
                vector<NodeInfo> node_info_vec;
                top_dir_node->children(node_info_vec);
                node_token_stack.push_back(node_info_vec);
            }
/**
 * Print like below:
 * .
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
            while (!node_token_stack.empty()) {
                auto &last_level = node_token_stack.back();

                while (!last_level.empty()) {
                    auto &curr_node_info = last_level.front();
                    bool is_last_node = last_level.size() == 1;
                    const char *curr_indent_str = is_last_node ? "`-- " : "|-- ";
                    ss << indent_str << curr_indent_str << curr_node_info.name;

                    auto curr_node = nodes_.at(curr_node_info.token);
                    if (curr_node->type() == NodeType::kFunc) {
                        ss << "*\r\n";
                    } else if (curr_node->type() == NodeType::kDir) {
                        //! 查重，防止循环路径引起的死循环
                        bool is_repeat = false;
                        for (size_t i = 0; i < node_token_stack.size() - 1; ++i) {
                            if (node_token_stack[i].front().token == curr_node_info.token) {
                                is_repeat = true;
                                ss << "(R)";
                                break;
                            }
                        }
                        ss << "\r\n";

                        auto curr_dir_node = static_cast<DirNode*>(curr_node);
                        vector<NodeInfo> node_info_vec;
                        curr_dir_node->children(node_info_vec);

                        if (!is_repeat && !node_info_vec.empty()) {
                            node_token_stack.push_back(node_info_vec);
                            indent_str += (is_last_node ? "    " : "|   ");
                            break;
                        }
                    }

                    //! 向上清理
                    while (!node_token_stack.empty()) {
                        node_token_stack.back().erase(node_token_stack.back().begin());
                        if (node_token_stack.back().empty()) {
                            node_token_stack.pop_back();
                            if (!indent_str.empty())
                                indent_str.erase(indent_str.size() - 4);
                        } else
                            break;
                    }
                }
            }
        } else {
            ss << " is a function\r\n";
        }
        is_succ = true;
    } else {
        ss << "Error: cannot access '" << path_str << "'\r\n";
    }

    s->send(ss.str());
    return is_succ;
}

void Terminal::Impl::executePwdCmd(SessionImpl *s, const Args &args)
{
    stringstream ss;
    ss << '/';
    for (size_t i = 0; i < s->path.size(); ++i) {
        ss << s->path.at(i).first;
        if ((i + 1) != s->path.size())
            ss << '/';
    }
    ss << "\r\n";
    s->send(ss.str());
}

bool Terminal::Impl::executeUserCmd(SessionImpl *s, const Args &args)
{
    stringstream ss;

    const auto &cmd = args[0];
    auto node_path = s->path;
    bool is_cmd_found = findNode(cmd, node_path);

    if (is_cmd_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        if (top_node->type() == NodeType::kFunc) {
            auto top_func_node = static_cast<FuncNode*>(top_node);
            return top_func_node->execute(*s, args);
        } else {
            s->path = node_path;
            return true;
        }
    } else {
        ss << "Error: " << cmd << " not found\r\n";
    }

    s->send(ss.str());
    return false;
}

}
