#include "json_to_graphviz.h"

#include <sstream>
#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/util/json.h>
#include <tbox/util/string.h>

namespace tbox {
namespace flow {

namespace {

int ActionJsonToGraphviz(const Json &js, std::ostringstream &oss)
{
    int id = 0;
    std::string type, state, result;

    if (!util::json::GetField(js, "id", id) ||
        !util::json::GetField(js, "type", type) ||
        !util::json::GetField(js, "state", state) ||
        !util::json::GetField(js, "result", result)) {
        return 0;
    }

    bool has_child = util::json::HasObjectField(js, "child");
    bool has_children_array = util::json::HasArrayField(js, "children");
    bool has_children_object = util::json::HasObjectField(js, "children");

    oss << "action_" << id << R"( [style="filled",)";
    if (state == "finished") {
        if (result == "success")
            oss << R"(fillcolor="green",)";
        else if (result == "fail")
            oss << R"(fillcolor="red",)";
    } else if (state == "running") {
        oss << R"(fillcolor="orange",)";
    } else if (state == "stoped") {
        oss << R"(fillcolor="gray",)";
    } else if (state == "pause") {
        oss << R"(fillcolor="lightblue",)";
    } else if (state == "idle") {
        oss << R"(fillcolor="white",)";
    } else {
        LogWarn("unsupport state: %s", state.c_str());
    }

    if (has_child || has_children_object || has_children_array)
        oss << R"(shape="rect",)";

    oss << R"(label=")";
    {
        oss << id << '.' << type;

        std::string label;
        if (util::json::GetField(js, "label", label))
            oss << R"(\n[)" << label << "]";

        for (auto &js_item : js.items()) {
            auto &key = js_item.key();
            if (key == "id" || key == "type" || key == "label" ||
                key == "child" || key == "children" ||
                key == "state" || key == "result")
                continue;
            std::string value_str = js_item.value().dump();
            util::string::Replace(value_str, R"(")", R"(\")");
            oss << R"(\n)" << key << " : " << value_str;
        }
    }
    oss << R"(")";
    oss << "];" << std::endl;

    if (has_child) {
        auto &js_child = js.at("child");
        int child_id = ActionJsonToGraphviz(js_child, oss);
        if (child_id > 0)
            oss << "action_" << id << "->action_" << child_id << ";" << std::endl;

    } else if (has_children_object) {
        auto &js_children_object = js.at("children");
        for (auto &js_child_item : js_children_object.items()) {
            int child_id = ActionJsonToGraphviz(js_child_item.value(), oss);
            if (child_id > 0)
                oss << "action_" << id << "->action_" << child_id << R"( [label=")" << js_child_item.key() << R"("];)" << std::endl;
        }
    } else if (has_children_array) {
        auto &js_children_array = js.at("children");
        for (size_t i = 0; i < js_children_array.size(); ++i) {
            auto &js_child = js_children_array.at(i);
            int child_id = ActionJsonToGraphviz(js_child, oss);
            if (child_id > 0)
                oss << "action_" << id << "->action_" << child_id << R"( [label="[)" << i << R"(]"];)" << std::endl;
        }
    }

    return id;
}

}

std::string ActionJsonToGraphviz(const Json &js)
{
    std::ostringstream oss;
    oss << "digraph {" << std::endl;
    ActionJsonToGraphviz(js, oss);
    oss << '}' << std::endl;
    return oss.str();
}

std::string StateMachineJsonToGraphviz(const Json &js)
{
    LogUndo();
    return "";
}

}
}
