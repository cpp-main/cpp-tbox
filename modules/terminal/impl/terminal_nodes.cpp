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

#include <tbox/base/log.h>
#include <tbox/util/string.h>

#include "dir_node.h"
#include "func_node.h"

namespace tbox {
namespace terminal {

using namespace std;

NodeToken Terminal::Impl::createFuncNode(const Func &func, const string &help)
{
    FuncNode *node = new FuncNode(func, help);
    return nodes_.alloc(node);
}

NodeToken Terminal::Impl::createDirNode(const string &help)
{
    DirNode *node = new DirNode(help);
    return nodes_.alloc(node);
}

bool Terminal::Impl::deleteNode(NodeToken node_token)
{
    auto node = nodes_.free(node_token);
    if (node != nullptr) {
        delete node;
        return true;
    } else {
        LogWarn("node not exist.");
        return false;
    }
}

NodeToken Terminal::Impl::findNode(const string &path_str) const
{
    Path node_path;
    if (findNode(path_str, node_path)) {
        if (node_path.empty())
            return root_token_;
        else
            return node_path.back().second;
    } else {
        return NodeToken();
    }
}

bool Terminal::Impl::mountNode(const NodeToken &parent, const NodeToken &child, const string &name)
{
    auto p_node = nodes_.at(parent);
    auto c_node = nodes_.at(child);

    if (p_node == nullptr || c_node == nullptr ||
        p_node->type() != NodeType::kDir) {
        LogWarn("mount '%s' fail, parent or child is invalid", name.c_str());
        return false;
    }

    if (name.empty() || name[0] == '!') {
        LogWarn("name '%s' is invalid", name.c_str());
        return false;
    }

    auto p_dir_node = static_cast<DirNode*>(p_node);
    return p_dir_node->addChild(name, child);
}

bool Terminal::Impl::umountNode(const NodeToken &parent, const std::string &name)
{
    auto p_node = nodes_.at(parent);

    if (p_node == nullptr ||
        p_node->type() != NodeType::kDir) {
        LogWarn("umount '%s' fail, parent is invalid", name.c_str());
        return false;
    }

    auto p_dir_node = static_cast<DirNode*>(p_node);
    if (!p_dir_node->removeChild(name)) {
        LogWarn("umount '%s' fail, not exist", name.c_str());
        return false;
    }

    return true;
}

bool Terminal::Impl::findNode(const string &path_str, Path &node_path) const
{
    vector<string> path_str_vec;
    util::string::Split(path_str, "/", path_str_vec);

    //! 如果是以 '/' 开头的路径
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

            if (top_node == nullptr)
                return false;

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
}
