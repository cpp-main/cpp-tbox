#include "terminal.h"

#include <sstream>

#include <tbox/base/log.h>
#include <tbox/util/string.h>

#include "session_imp.h"
#include "dir_node.h"
#include "func_node.h"

namespace tbox::terminal {

using namespace std;

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
        "Buildin commands:\r\n"
        "- cd        Chang directory\r\n"
        "- ls        List nodes under specified path\r\n"
        "- tree      List all nodes as tree\r\n"
        "- exit      Exit this\r\n"
        "- help      Print help of specified node\r\n"
        "- history   List history command\r\n"
        ;
    s->send(help_str);
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
