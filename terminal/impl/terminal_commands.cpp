#include "terminal.h"

#include <iomanip>
#include <sstream>

#include <tbox/base/log.h>
#include <tbox/util/split_cmdline.h>

#include "session_context.h"
#include "dir_node.h"
#include "func_node.h"

namespace tbox::terminal {

using namespace std;

bool Terminal::Impl::executeCmdline(SessionContext *s)
{
    auto cmdline = s->curr_input;
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
    } else if (cmd == "exit") {
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
        if (top_node->type() == NodeType::kDir) {
            s->path = node_path;
        } else {
            ss << "Error: '" << path_str << "' not directory" << "\r\n";
        }
    } else {
        ss << "Error: cannot access '" << path_str << "'\r\n";
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
        ss << top_node->help() << "\r\n";
    } else {
        ss << "Error: cannot access '" << path_str << "'\r\n";
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
        if (top_node->type() == NodeType::kDir) {
            auto top_dir_node = static_cast<DirNode*>(top_node);
            vector<NodeInfo> node_info_vec;
            top_dir_node->children(node_info_vec);

            for (auto item : node_info_vec) {
                ss << item.name;
                auto node = nodes_.at(item.token);
                if (node->type() == NodeType::kFunc)
                    ss << '*';
                ss << '\t';
            }

            ss << "\r\n";
        } else {
            ss << path_str << " is function" << "\r\n";
        }
    } else {
        ss << "Error: cannot access '" << path_str << "'\r\n";
    }

    s->wp_conn->send(s->token, ss.str());
}

void Terminal::Impl::executeHistoryCmd(SessionContext *s, const Args &args)
{
    stringstream ss;
    for (size_t i = 0; i < s->history.size(); ++i) {
        const auto &cmd = s->history.at(i);
        ss << setw(2) << i << "  " << cmd << "\r\n";
    }
    s->wp_conn->send(s->token, ss.str());
}

void Terminal::Impl::executeExitCmd(SessionContext *s, const Args &args)
{
    s->wp_conn->send(s->token, "Bye!\r\n");
    s->wp_conn->endSession(s->token);
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
        if (top_node->type() == NodeType::kDir) {
            vector<vector<NodeInfo>> node_token_stack;    //! 遍历记录
            string indent_str;

            {
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
            while (!node_token_stack.empty()) {
                auto &last_level = node_token_stack.back();

                while (!last_level.empty()) {
                    bool is_last_node = last_level.size() == 1;
                    const char *curr_indent_str = is_last_node ? "`-- " : "|-- ";

                    auto &curr_node_info = last_level.front();
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
                                ss << "(R)";    //! 表示循环引用了
                                break;
                            }
                        }
                        ss << "\r\n";

                        if (!is_repeat) {
                            auto curr_dir_node = static_cast<DirNode*>(curr_node);
                            vector<NodeInfo> node_info_vec;
                            curr_dir_node->children(node_info_vec);
                            if (!node_info_vec.empty()) {
                                //! 向下延伸
                                node_token_stack.push_back(node_info_vec);
                                indent_str += (is_last_node ? "    " : "|   ");
                                break;
                            }
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
            ss << node_path.back().first << " is a function\r\n";
        }
    } else {
        ss << "Error: cannot access '" << path_str << "'\r\n";
    }

    s->wp_conn->send(s->token, ss.str());
}

void Terminal::Impl::executePwdCmd(SessionContext *s, const Args &args)
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
        return executeCmdline(s);
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
            return executeCmdline(s);
        } else
            s->wp_conn->send(s->token, "Error: index out of range\r\n");
    } catch (const invalid_argument &e) {
        s->wp_conn->send(s->token, "Error: parse index fail\r\n");
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
        if (top_node->type() == NodeType::kFunc) {
            auto top_func_node = static_cast<FuncNode*>(top_node);
            Session session(s->wp_conn, s->token);
            top_func_node->execute(session, args);
        } else {
            s->path = node_path;
        }
    } else {
        ss << "Error: " << cmd << " not found\r\n";
    }

    s->wp_conn->send(s->token, ss.str());
}

}
