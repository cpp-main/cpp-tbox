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
            //!TODO
        } else {
            //!TODO
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
