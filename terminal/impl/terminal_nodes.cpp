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

    if (p_node == nullptr || c_node == nullptr ||
        p_node->type() != NodeType::kDir) {
        LogWarn("parent or child is invalid, or parent not directory");
        return false;
    }

    if (name.empty() || name[0] == '!') {
        LogWarn("name is invalid");
        return false;
    }

    auto p_dir_node = static_cast<DirNode*>(p_node);
    return p_dir_node->addChild(child, name);
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
}
