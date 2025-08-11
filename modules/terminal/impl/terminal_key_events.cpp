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

#include <sstream>

#include <tbox/base/log.h>
#include <tbox/util/string.h>

#include "session_context.h"
#include "dir_node.h"
#include "func_node.h"
#include "../connection.h"

namespace tbox {
namespace terminal {

using namespace std;

namespace {
    const string MOVE_LEFT_KEY("\033[D");
    const string MOVE_RIGHT_KEY("\033[C");
    const size_t HISTORY_MAX_SIZE(20);
    const char * SPACE_CHARS = " \t";
}

void Terminal::Impl::onChar(SessionContext *s, char ch)
{
    if (s->cursor_pos == s->curr_input.size())
        s->curr_input.push_back(ch);
    else
        s->curr_input.insert(s->cursor_pos, 1, ch);
    s->cursor_pos++;

    if (s->options & kEnableEcho) {
        s->wp_conn->send(s->token, ch);

        ostringstream oss;
        oss << s->curr_input.substr(s->cursor_pos)
            << string((s->curr_input.size() - s->cursor_pos), '\b');

        auto refresh_str = oss.str();
        if (!refresh_str.empty())
            s->wp_conn->send(s->token, refresh_str);
    }
}

void Terminal::Impl::onEnterKey(SessionContext *s)
{
    if (s->options & kEnableEcho)
        s->wp_conn->send(s->token, "\r\n");

    if (execute(s)) {
        s->history.push_back(s->curr_input);
        if (s->history.size() > HISTORY_MAX_SIZE)
            s->history.pop_front();
    }

    printPrompt(s);

    s->curr_input.clear();
    s->cursor_pos = 0;
    s->history_index = 0;
}

void Terminal::Impl::onBackspaceKey(SessionContext *s)
{
    if (s->cursor_pos == 0)
        return;

    if (s->cursor_pos == s->curr_input.size())
        s->curr_input.pop_back();
    else
        s->curr_input.erase((s->cursor_pos - 1), 1);

    s->cursor_pos--;

    if (s->options & kEnableEcho) {
        ostringstream oss;
        oss << '\b' << s->curr_input.substr(s->cursor_pos) << ' '
            << string((s->curr_input.size() - s->cursor_pos + 1), '\b');

        s->wp_conn->send(s->token, oss.str());
    }
}

void Terminal::Impl::onDeleteKey(SessionContext *s)
{
    if (s->cursor_pos >= s->curr_input.size())
        return;

    s->curr_input.erase((s->cursor_pos), 1);

    if (s->options & kEnableEcho) {
        ostringstream oss;
        oss << s->curr_input.substr(s->cursor_pos) << ' '
            << string((s->curr_input.size() - s->cursor_pos + 1), '\b');

        s->wp_conn->send(s->token, oss.str());
    }
}

/**
 * 仅实现对Node的补全，不实现参数的补全
 */
void Terminal::Impl::onTabKey(SessionContext *s)
{
    //! 如果当前用户没有任何输入，则不进行补全
    if (s->curr_input.empty() || s->cursor_pos == 0)
        return;

    //! 找出当前光标下正在输入的命令的起始位置
    size_t start_pos = 0;
    {
        /**
         * 针对单行多指令的情况："set_a 12; get_ "
         *                                      ^
         * 要补全的是get_，而不是set_a
         */

        //! 尝试从光标所在处往前寻找分号。如果是找到，则将start_pos修正为分号后一位
        auto semicolon_pos = s->curr_input.find_last_of(';', s->cursor_pos - 1);
        if (semicolon_pos != std::string::npos)
            start_pos = semicolon_pos + 1;

        //! 略过左边空格，找到有效字符起始位置
        start_pos = s->curr_input.find_first_not_of(SPACE_CHARS, start_pos);

        /**
         * 如果没有找到或找到的位置在光标之后，说明无效，则退出
         * 如："  set_a 12"
         *       ^
         */
        if (start_pos == std::string::npos || start_pos >= s->cursor_pos)
            return;
    }

    const auto cmd_str = s->curr_input.substr(start_pos, s->cursor_pos - start_pos);

    if (cmd_str.empty())
        return;

    /**
     * 检查从start_pos到s->cursor_pos之间有没有空格
     * 如果有则说明当前用户输入的是参数，不是命令，不进行补全
     * 如："set_value 12"
     *                ^
     */
    {
        auto space_pos = cmd_str.find_first_of(SPACE_CHARS);
        if (space_pos != std::string::npos)
            return;
    }

    //! 分离路径和节点名
    size_t last_slash_pos = cmd_str.rfind('/');
    string dir_path;
    string prefix;

    if (last_slash_pos != string::npos) {
        dir_path = cmd_str.substr(0, last_slash_pos);
        prefix = cmd_str.substr(last_slash_pos + 1);
    } else {
        dir_path = ".";
        prefix = cmd_str;
    }

    //! 查找路径对应的节点
    NodeToken dir_token = findNode(dir_path, s);
    if (dir_token.isNull())
        return;

    //! 获取所有匹配的子节点对象
    Node* base_node = nodes_.at(dir_token);
    if (base_node == nullptr)
        return;

    vector<string> matched_node_name_vec;
    if (auto dir_node = dynamic_cast<DirNode*>(base_node)) {
        vector<NodeInfo> children_info_vec;
        dir_node->children(children_info_vec);

        //! 遍历dir_node下有所有子结点，找出匹配得上的，加入到matched_node_name_vec中
        for (const auto& child_info : children_info_vec) {
            if (util::string::IsStartWith(child_info.name, prefix))
                matched_node_name_vec.push_back(child_info.name);
        }
    }

    //! 如果没有找到匹配项，直接结束
    if (matched_node_name_vec.empty())
        return;

    string completion;  //! 补全内容

    //! 如果只有一个匹配，直接补全
    if (matched_node_name_vec.size() == 1) {
        const auto &match_node_name = matched_node_name_vec.back();
        completion = match_node_name.substr(prefix.length());    //! 需要补全的字串

        //! 如果不需要补全，则结束
        if (completion.empty())
            return;

        //! 检查是否为目录，如果是则添加/
        NodeToken child_token = findNode(dir_path + "/" + match_node_name, s);
        if (!child_token.isNull()) {
            Node* child_node = nodes_.at(child_token);
            if (child_node && child_node->type() == NodeType::kDir) {
                completion.push_back('/');
            }
        }

    } else {
        //! 多个匹配，显示所有可能
        ostringstream oss;
        oss << "\r\n";
        for (const auto& match : matched_node_name_vec)
            oss << match << "\t";
        oss << "\r\n";

        if (s->options & kEnableEcho) {
            s->wp_conn->send(s->token, oss.str());
            printPrompt(s);
            s->wp_conn->send(s->token, s->curr_input);
        }

        //! 找出最大共同字串，设置补全内容
        std::string common_prefix = util::string::ExtractCommonPrefix(matched_node_name_vec);
        completion = common_prefix.substr(prefix.length()); //! 需要补全的字串
    }

    //! 如果需要补全，则进行补全
    if (!completion.empty()) {
        s->curr_input.insert(s->cursor_pos, completion);
        s->cursor_pos += completion.length();

        if (s->options & kEnableEcho) {
            s->wp_conn->send(s->token, completion);

            ostringstream oss;
            oss << s->curr_input.substr(s->cursor_pos)
                << string((s->curr_input.size() - s->cursor_pos), '\b');

            auto refresh_str = oss.str();
            if (!refresh_str.empty())
                s->wp_conn->send(s->token, refresh_str);
        }
    }
}

namespace {
void CleanupInput(SessionContext *s)
{
    while (s->cursor_pos < s->curr_input.size()) {
        s->wp_conn->send(s->token, MOVE_RIGHT_KEY);
        s->cursor_pos++;
    }

    while (s->cursor_pos--)
        s->wp_conn->send(s->token, "\b \b");
}
}

void Terminal::Impl::onMoveUpKey(SessionContext *s)
{
    if (s->history_index == s->history.size())
        return;

    CleanupInput(s);

    s->history_index++;
    s->curr_input = s->history[s->history.size() - s->history_index];
    s->cursor_pos = s->curr_input.size();

    s->wp_conn->send(s->token, s->curr_input);
}

void Terminal::Impl::onMoveDownKey(SessionContext *s)
{
    if (s->history_index == 0)
        return;

    CleanupInput(s);

    s->history_index--;
    if (s->history_index > 0) {
        s->curr_input = s->history[s->history.size() - s->history_index];
        s->cursor_pos = s->curr_input.size();
    } else {
        s->curr_input.clear();
        s->cursor_pos = 0;
    }

    s->wp_conn->send(s->token, s->curr_input);
}

void Terminal::Impl::onMoveLeftKey(SessionContext *s)
{
    if (s->cursor_pos == 0)
        return;

    s->cursor_pos--;
    s->wp_conn->send(s->token, MOVE_LEFT_KEY);
}

void Terminal::Impl::onMoveRightKey(SessionContext *s)
{
    if (s->cursor_pos >= s->curr_input.size())
        return;

    s->cursor_pos++;
    s->wp_conn->send(s->token, MOVE_RIGHT_KEY);
}

void Terminal::Impl::onHomeKey(SessionContext *s)
{
    while (s->cursor_pos != 0) {
        s->wp_conn->send(s->token, MOVE_LEFT_KEY);
        s->cursor_pos--;
    }
}

void Terminal::Impl::onEndKey(SessionContext *s)
{
    while (s->cursor_pos < s->curr_input.size()) {
        s->wp_conn->send(s->token, MOVE_RIGHT_KEY);
        s->cursor_pos++;
    }
}

}
}
