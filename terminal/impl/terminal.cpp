#include "terminal.h"

#include <vector>
#include <deque>
#include <sstream>

#include <tbox/base/log.h>

#include "session_imp.h"

namespace tbox::terminal {

namespace {
const std::string MOVE_LEFT_KEY("\033[D");
const std::string MOVE_RIGHT_KEY("\033[C");
const size_t HISTORY_MAX_SIZE(20);
}

Terminal::Impl::~Impl()
{
    sessions_.foreach(
        [](SessionImpl *p) {
            delete p;
        }
    );
    sessions_.clear();
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

bool Terminal::Impl::onRecvString(const SessionToken &st, const std::string &str)
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

NodeToken Terminal::Impl::create(const EndNode &info)
{
    LogUndo();
    return NodeToken();
}

NodeToken Terminal::Impl::create(const DirNode &info)
{
    LogUndo();
    return NodeToken();
}

NodeToken Terminal::Impl::root() const
{
    LogUndo();
    return NodeToken();
}

NodeToken Terminal::Impl::find(const std::string &path) const
{
    LogUndo();
    return NodeToken();
}

bool Terminal::Impl::mount(const NodeToken &parent, const NodeToken &child)
{
    LogUndo();
    return false;
}

void Terminal::Impl::onChar(SessionImpl *s, char ch)
{
    s->send(ch);

    if (s->cursor == s->curr_input.size())
        s->curr_input.push_back(ch);
    else
        s->curr_input.insert(s->cursor, 1, ch);
    s->cursor++;

    std::stringstream ss;
    ss  << s->curr_input.substr(s->cursor)
        << std::string((s->curr_input.size() - s->cursor), '\b');
    s->send(ss.str());
}

void Terminal::Impl::onEnterKey(SessionImpl *s)
{
    s->send("\r\n");

    bool is_succ = executeCmd(s);

    printPrompt(s);

    if (is_succ) {
        //! 如果成功，则将已执行的命令加入history，另起一行
        s->cursor = 0;
        s->history.push_back(std::move(s->curr_input));
        if (s->history.size() > HISTORY_MAX_SIZE)
            s->history.pop_front();
    } else {
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

    std::stringstream ss;
    ss  << '\b' << s->curr_input.substr(s->cursor) << ' '
        << std::string((s->curr_input.size() - s->cursor + 1), '\b');
    s->send(ss.str());
}

void Terminal::Impl::onDeleteKey(SessionImpl *s)
{
    if (s->cursor >= s->curr_input.size())
        return;

    s->curr_input.erase((s->cursor), 1);

    std::stringstream ss;
    ss  << s->curr_input.substr(s->cursor) << ' '
        << std::string((s->curr_input.size() - s->cursor + 1), '\b');
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
    using namespace std;
    stringstream ss;
    //!TODO:打印当前路径
    ss << "> ";
    s->send(ss.str());
}

bool Terminal::Impl::executeCmd(SessionImpl *s)
{
    //!TODO:处理并执行命令
    LogUndo();
    return true;
}
}
