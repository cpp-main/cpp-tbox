#include "terminal.h"

#include <sstream>

#include <tbox/base/log.h>

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

}
