#ifndef TBOX_FLOW_TO_GRAPHVIZ_H_20230323
#define TBOX_FLOW_TO_GRAPHVIZ_H_20230323

#include <base/json_fwd.h>

namespace tbox {
namespace flow {

class Action;
class StateMachine;

/// 将 Action 的 Json 以 Graphviz 文本输出
std::string ActionJsonToGraphviz(const Json &js);

/// 将 StateMachine 的 Json 以 Graphviz 文本输出
std::string StateMachineJsonToGraphviz(const Json &js);

/// 将 Action 以 Graphviz 文本输出
std::string ToGraphviz(const Action &action);
std::string ToGraphviz(const Action *action);

/// 将 StateMachine 以 Graphviz 文本输出
std::string ToGraphviz(const StateMachine &sm);
std::string ToGraphviz(const StateMachine *sm);

}
}

#endif //TBOX_FLOW_TO_GRAPHVIZ_H_20230323
