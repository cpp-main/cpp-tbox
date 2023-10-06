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
#include "dir_node.h"

namespace tbox {
namespace terminal {

bool DirNode::addChild(const std::string &child_name, const NodeToken &nt)
{
    auto iter = children_.find(child_name);
    if (iter != children_.end())
        return false;

    children_.insert(std::make_pair(child_name, nt));
    return true;
}

bool DirNode::removeChild(const std::string &child_name)
{
    auto iter = children_.find(child_name);
    if (iter == children_.end())
        return false;

    children_.erase(iter);
    return true;
}

NodeToken DirNode::findChild(const std::string &child_name) const
{
    auto iter = children_.find(child_name);
    if (iter == children_.end())
        return NodeToken();
    return iter->second;
}

void DirNode::children(std::vector<NodeInfo> &vec) const
{
    for (auto item : children_) {
        NodeInfo info{ item.first, item.second };
        vec.push_back(info);
    }
}

}
}
