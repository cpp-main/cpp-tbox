#include "terminal.h"

#include <vector>
#include <deque>
#include <sstream>
#include <iomanip>

#include <tbox/base/log.h>
#include <tbox/util/split_cmdline.h>
#include <tbox/util/string.h>

#include "session_imp.h"
#include "dir_node.h"
#include "func_node.h"

namespace tbox::terminal {

using namespace std;

namespace {
    const string MOVE_LEFT_KEY("\033[D");
    const string MOVE_RIGHT_KEY("\033[C");
    const size_t HISTORY_MAX_SIZE(20);
}

Terminal::Impl::Impl()
{
    root_token_ = nodes_.insert(new DirNode("this is root node"));
}

Terminal::Impl::~Impl()
{
    sessions_.foreach(
        [](SessionImpl *p) {
            delete p;
        }
    );
    sessions_.clear();

    nodes_.foreach(
        [](Node *p) {
            delete p;
        }
    );
    nodes_.clear();
}

SessionToken Terminal::Impl::newSession(Connection *wp_conn)
{
    auto s = new SessionImpl(wp_conn);
    auto t = sessions_.insert(s);
    s->setSessionToken(t);
    return t;
}

bool Terminal::Impl::deleteSession(const SessionToken &st)
{
    auto s = sessions_.remove(st);
    if (s != nullptr) {
        delete s;
        return true;
    }
    return false;
}

bool Terminal::Impl::onBegin(const SessionToken &st)
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return false;

    s->send("\r\nWelcome to TBox Terminal.\r\n");
    printPrompt(s);

    return true;
}

bool Terminal::Impl::onExit(const SessionToken &st)
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return false;

    s->send("Bye!");
    return true;
}

bool Terminal::Impl::onRecvString(const SessionToken &st, const string &str)
{
    auto s = sessions_.at(st);
    if (s == nullptr)
        return false;

    s->key_event_scanner_.start();
    for (char c : str) {
        auto status = s->key_event_scanner_.next(c);
        if (status == KeyEventScanner::Status::kEnsure) {
            switch (s->key_event_scanner_.result()) {
                case KeyEventScanner::Result::kPrintable:
                    onChar(s, c);
                    break;
                case KeyEventScanner::Result::kEnter:
                    onEnterKey(s);
                    break;
                case KeyEventScanner::Result::kBackspace:
                    onBackspaceKey(s);
                    break;
                case KeyEventScanner::Result::kTab:
                    onTabKey(s);
                    break;
                case KeyEventScanner::Result::kMoveUp:
                    onMoveUpKey(s);
                    break;
                case KeyEventScanner::Result::kMoveDown:
                    onMoveDownKey(s);
                    break;
                case KeyEventScanner::Result::kMoveLeft:
                    onMoveLeftKey(s);
                    break;
                case KeyEventScanner::Result::kMoveRight:
                    onMoveRightKey(s);
                    break;
                case KeyEventScanner::Result::kHome:
                    onHomeKey(s);
                    break;
                case KeyEventScanner::Result::kEnd:
                    onEndKey(s);
                    break;
                case KeyEventScanner::Result::kDelete:
                    onDeleteKey(s);
                    break;
                default:
                    break;
            }
            s->key_event_scanner_.start();
        }
    }
    s->key_event_scanner_.stop();
    return true;
}

bool Terminal::Impl::onRecvWindowSize(const SessionToken &st, uint16_t w, uint16_t h)
{
    auto s = sessions_.at(st);
    if (s != nullptr) {
        s->setWindowSize(w, h);
        return true;
    }
    return false;
}

NodeToken Terminal::Impl::createFuncNode(const Func &func, const string &help)
{
    FuncNode *node = new FuncNode(func, help);
    return nodes_.insert(node);
}

NodeToken Terminal::Impl::createDirNode(const string &help)
{
    DirNode *node = new DirNode(help);
    return nodes_.insert(node);
}

NodeToken Terminal::Impl::rootNode() const
{
    return root_token_;
}

NodeToken Terminal::Impl::findNode(const string &path_str) const
{
    Path node_path;
    bool is_found = findNode(path_str, node_path);
    return is_found ? node_path.back().second : NodeToken();
}

bool Terminal::Impl::mountNode(const NodeToken &parent, const NodeToken &child, const string &name)
{
    auto p_node = nodes_.at(parent);
    auto c_node = nodes_.at(child);

    if (p_node == nullptr || c_node == nullptr)
        return false;

    if (p_node->type() != NodeType::kDir)
        return false;

    auto p_dir_node = static_cast<DirNode*>(p_node);
    return p_dir_node->addChild(child, name);
}

void Terminal::Impl::onChar(SessionImpl *s, char ch)
{
    s->send(ch);

    if (s->cursor == s->curr_input.size())
        s->curr_input.push_back(ch);
    else
        s->curr_input.insert(s->cursor, 1, ch);
    s->cursor++;

    stringstream ss;
    ss  << s->curr_input.substr(s->cursor)
        << string((s->curr_input.size() - s->cursor), '\b');
    s->send(ss.str());
}

void Terminal::Impl::onEnterKey(SessionImpl *s)
{
    s->send("\r\n");

    bool store_in_history = false;
    bool recover_cmdline = false;
    executeCmdline(s, store_in_history, recover_cmdline);

    printPrompt(s);

    if (store_in_history) {
        //! 如果成功，则将已执行的命令加入history，另起一行
        s->cursor = 0;
        s->history.push_back(move(s->curr_input));
        if (s->history.size() > HISTORY_MAX_SIZE)
            s->history.pop_front();
    }

    if (recover_cmdline) {
        //! 如果没有成功，则将原来的命令重新打印出来
        s->cursor = s->curr_input.size();
        s->send(s->curr_input);
    }

    s->history_index = 0;
}

void Terminal::Impl::onBackspaceKey(SessionImpl *s)
{
    if (s->cursor == 0)
        return;

    if (s->cursor == s->curr_input.size())
        s->curr_input.pop_back();
    else
        s->curr_input.erase((s->cursor - 1), 1);

    s->cursor--;

    stringstream ss;
    ss  << '\b' << s->curr_input.substr(s->cursor) << ' '
        << string((s->curr_input.size() - s->cursor + 1), '\b');
    s->send(ss.str());
}

void Terminal::Impl::onDeleteKey(SessionImpl *s)
{
    if (s->cursor >= s->curr_input.size())
        return;

    s->curr_input.erase((s->cursor), 1);

    stringstream ss;
    ss  << s->curr_input.substr(s->cursor) << ' '
        << string((s->curr_input.size() - s->cursor + 1), '\b');
    s->send(ss.str());
}

void Terminal::Impl::onTabKey(SessionImpl *s)
{
    //!TODO: 实现补全功能
    LogUndo();
}

void Terminal::Impl::onMoveUpKey(SessionImpl *s)
{
    if (s->history_index == s->history.size())
        return;

    while (s->cursor--)
        s->send("\b \b");

    s->history_index++;
    s->curr_input = s->history[s->history.size() - s->history_index];
    s->cursor = s->curr_input.size();
    s->send(s->curr_input);
}

void Terminal::Impl::onMoveDownKey(SessionImpl *s)
{
    if (s->history_index == 0)
        return;

    while (s->cursor--)
        s->send("\b \b");

    s->history_index--;
    if (s->history_index > 0) {
        s->curr_input = s->history[s->history.size() - s->history_index];
        s->cursor = s->curr_input.size();
    } else {
        s->curr_input.clear();
        s->cursor = 0;
    }
    s->send(s->curr_input);
}

void Terminal::Impl::onMoveLeftKey(SessionImpl *s)
{
    if (s->cursor == 0)
        return;

    s->cursor--;
    s->send(MOVE_LEFT_KEY);
}

void Terminal::Impl::onMoveRightKey(SessionImpl *s)
{
    if (s->cursor >= s->curr_input.size())
        return;

    s->cursor++;
    s->send(MOVE_RIGHT_KEY);
}

void Terminal::Impl::onHomeKey(SessionImpl *s)
{
    while (s->cursor != 0) {
        s->send(MOVE_LEFT_KEY);
        s->cursor--;
    }
}

void Terminal::Impl::onEndKey(SessionImpl *s)
{
    while (s->cursor < s->curr_input.size()) {
        s->send(MOVE_RIGHT_KEY);
        s->cursor++;
    }
}

void Terminal::Impl::printPrompt(SessionImpl *s)
{
    stringstream ss;
    ss << '/';
    for (size_t i = 0; i < s->path.size(); ++i) {
        ss << s->path.at(i).first;
        if ((i + 1) != s->path.size())
            ss << '/';
    }
    ss << " # ";
    s->send(ss.str());
}

void Terminal::Impl::printHelp(SessionImpl *s)
{
    const char *help_str = \
        "buildin command:\r\n"
        "- cd        Chang directory\r\n"
        "- ls        List nodes under specified path\r\n"
        "- tree      List all nodes as tree\r\n"
        "- exit      Exit this\r\n"
        "- help      Print help of specified node\r\n"
        "- history   List history command\r\n"
        ;
    s->send(help_str);
}

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

    store_in_history = false;
    recover_cmdline = false;

    const auto &cmd = args[0];
    if (cmd == "ls") {
        bool is_succ = executeLsCmd(s, args);
        store_in_history = is_succ;
        recover_cmdline = !is_succ;
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
        executeTreeCmd(s, args);
        store_in_history = true;
        recover_cmdline = false;
    } else {
        bool is_succ = executeUserCmd(s, args);
        store_in_history = is_succ;
        recover_cmdline = !is_succ;
    }
}

bool Terminal::Impl::executeCdCmd(SessionImpl *s, const Args &args)
{
    string path = "/";
    if (args.size() >= 2)
        path = args[1];

    auto node_path = s->path;
    bool is_found = findNode(path, node_path);
    if (is_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        if (top_node->type() == NodeType::kDir) {
            s->path = node_path;
            return true;
        }
    }

    stringstream ss;
    ss << "Error: cannot access '" << path << "'\r\n";
    s->send(ss.str());
    return false;
}

bool Terminal::Impl::executeHelpCmd(SessionImpl *s, const Args &args)
{
    if (args.size() < 2) {
        printHelp(s);
        return true;
    }

    string path_str = args[1];
    auto node_path = s->path;
    bool is_found = findNode(path_str, node_path);
    if (is_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        auto help_str = top_node->help() + "\r\n";
        s->send(help_str);
        return true;
    }

    stringstream ss;
    ss << "Error: cannot access '" << path_str << "'\r\n";
    s->send(ss.str());
    return false;
}

bool Terminal::Impl::executeLsCmd(SessionImpl *s, const Args &args)
{
    string path = ".";
    if (args.size() >= 2)
        path = args[1];

    auto node_path = s->path;
    bool is_found = findNode(path, node_path);
    if (is_found) {
        auto top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
        auto top_node = nodes_.at(top_node_token);
        if (top_node->type() == NodeType::kDir) {
            auto top_dir_node = static_cast<DirNode*>(top_node);
            vector<NodeInfo> node_info_vec;
            top_dir_node->children(node_info_vec);

            stringstream ss;
            for (auto item : node_info_vec)
                ss << item.name << "\r\n";
            s->send(ss.str());

            return true;
        }
    }

    stringstream ss;
    ss << "Error: cannot access '" << path << "'\r\n";
    s->send(ss.str());
    return false;
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

void Terminal::Impl::executeTreeCmd(SessionImpl *s, const Args &args)
{
    LogUndo();
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

bool Terminal::Impl::findNode(const string &path_str, Path &node_path) const
{
    vector<string> path_str_vec;
    util::string::Split(path_str, "/", path_str_vec);

    if (path_str_vec[0].empty()) {
        node_path.clear();
        path_str_vec.erase(path_str_vec.begin());
    }

    for (const auto &node_name : path_str_vec) {
        if (node_name == "." || node_name.empty()) {
            continue;
        } else if (node_name == "..") {
            if (node_path.empty())
                return false;
            else
                node_path.pop_back();
        } else {
            NodeToken top_node_token = node_path.empty() ? root_token_ : node_path.back().second;
            Node *top_node = nodes_.at(top_node_token);
            if (top_node->type() == NodeType::kFunc)
                return false;

            DirNode *top_dir_node = static_cast<DirNode*>(top_node);
            auto next_node_token = top_dir_node->findChild(node_name);
            if (next_node_token.isNull())
                return false;

            node_path.push_back(make_pair(node_name, next_node_token));
        }
    }
    return true;
}

}
