#include "to_graphviz.h"

#include <sstream>
#include <tbox/base/log.h>
#include <tbox/base/json.hpp>
#include <tbox/base/assert.h>
#include <tbox/util/json.h>
#include <tbox/util/string.h>

#include "action.h"
#include "state_machine.h"

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

    oss << "action_" << id << R"( [)";
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
            oss << R"(\n)" << key << " = " << value_str;
        }
    }
    oss << R"(")";
    oss << "];" << std::endl;

    if (util::json::HasObjectField(js, "child")) {
        auto &js_child = js.at("child");
        int child_id = ActionJsonToGraphviz(js_child, oss);
        if (child_id > 0)
            oss << "action_" << id << "->action_" << child_id << ";" << std::endl;

    } else if (util::json::HasObjectField(js, "children")) {
        auto &js_children_object = js.at("children");
        for (auto &js_child_item : js_children_object.items()) {
            int child_id = ActionJsonToGraphviz(js_child_item.value(), oss);
            if (child_id > 0)
                oss << "action_" << id << "->action_" << child_id << R"( [label=")" << js_child_item.key() << R"("];)" << std::endl;
        }

    } else if (util::json::HasArrayField(js, "children")) {
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

void StateMachineJsonToGraphviz(const Json &js, std::ostringstream &oss, int &sm_id_alloc)
{
    int term_state = 0;
    int init_state = 0;
    int curr_state = -1;
    int curr_sm_id = ++sm_id_alloc;

    if (!util::json::GetField(js, "init_state", init_state) ||
        !util::json::GetField(js, "term_state", term_state))
        return;

    util::json::GetField(js, "curr_state", curr_state);

    if (!util::json::HasArrayField(js, "states"))
        return;

    const auto &js_state_array = js["states"];
    for (auto &js_state : js_state_array) {
        int state_id = 0;
        std::string label;
        if (!util::json::GetField(js_state, "id", state_id) ||
            !util::json::GetField(js_state, "label", label))
            continue;

        const char *curr_state_color = "orange";

        bool has_sub_sm = util::json::HasObjectField(js_state, "sub_sm");

        oss << "state_" << curr_sm_id << '_' << state_id << R"( [)";
        if (state_id == curr_state)
            oss << R"(fillcolor=")" << curr_state_color << R"(",)";

        if (has_sub_sm)
            oss << R"(shape="box3d",)";

        if (state_id == init_state) {
            oss << R"(shape="circle",label="")";
        } else {
            oss << R"(label=")" << state_id;
            if (!label.empty())
                oss << '.' << label;
            oss << R"(")";
        }
        oss << R"(];)" << std::endl;

        if (util::json::HasArrayField(js_state, "routes")) {
            auto &js_route_array = js_state["routes"];
            for (auto &js_route : js_route_array) {
                int event_id = 0;
                int next_state_id = 0;
                std::string label;
                if (!util::json::GetField(js_route, "event_id", event_id) ||
                    !util::json::GetField(js_route, "next_state_id", next_state_id) ||
                    !util::json::GetField(js_route, "label", label))
                    continue;
                oss << "state_" << curr_sm_id << '_' << state_id << "->"
                    << "state_" << curr_sm_id << '_' << next_state_id
                    << R"( [)";
                if (state_id == curr_state) {
                    oss << R"(color=")" << curr_state_color << R"(",)"
                        << R"(fontcolor=")" << curr_state_color << R"(",)";
                }
                oss << R"(label=")"
                    << next_state_id;

                if (!label.empty())
                    oss << '.' << label;
                oss << R"("];)" << std::endl;
            }
        }

        if (has_sub_sm) {
            auto &js_sub_sm = js_state["sub_sm"];
            oss << "subgraph cluster_" << state_id << " {" << std::endl
                << R"(style="rounded";)"
                << R"(label=")" << state_id;

            if (!label.empty())
                oss << '.' << label;

            oss << R"(";)" << std::endl;
            StateMachineJsonToGraphviz(js_sub_sm, oss, sm_id_alloc);
            oss << '}' << std::endl;
        }
    }

    oss << "state_" << curr_sm_id << '_' << 0
        << R"( [shape="doublecircle",style="filled",fillcolor="black",label=""];)" << std::endl;
}

}

std::string ActionJsonToGraphviz(const Json &js)
{
    std::ostringstream oss;
    oss << "digraph {" << std::endl
        << R"(node [shape="rect",style="filled"];)" << std::endl;
    ActionJsonToGraphviz(js, oss);
    oss << '}' << std::endl;
    return oss.str();
}

std::string StateMachineJsonToGraphviz(const Json &js)
{
    std::ostringstream oss;
    int sm_id_alloc = 0;

    oss << "digraph {" << std::endl
        << R"(node [shape="rect",style="filled,rounded"];)" << std::endl;
    StateMachineJsonToGraphviz(js, oss, sm_id_alloc);
    oss << '}' << std::endl;
    return oss.str();
    return "";
}

std::string ToGraphviz(const Action &action)
{
    Json js;
    action.toJson(js);
    return ActionJsonToGraphviz(js);
}

std::string ToGraphviz(const Action *action)
{
    TBOX_ASSERT(action != nullptr);

    Json js;
    action->toJson(js);
    return ActionJsonToGraphviz(js);
}

std::string ToGraphviz(const StateMachine &sm)
{
    Json js;
    sm.toJson(js);
    return StateMachineJsonToGraphviz(js);
}

std::string ToGraphviz(const StateMachine *sm)
{
    TBOX_ASSERT(sm != nullptr);

    Json js;
    sm->toJson(js);
    return StateMachineJsonToGraphviz(js);
}

}
}
