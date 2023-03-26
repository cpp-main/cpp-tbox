#ifndef TBOX_FLOW_JSON_TO_GRAPHVIZ_H_20230323
#define TBOX_FLOW_JSON_TO_GRAPHVIZ_H_20230323

#include <tbox/base/json_fwd.h>

namespace tbox {
namespace flow {

/// 将 Action 的 Json 输出以 Graphviz 文本输出
std::string ActionJsonToGraphviz(const Json &js);

/// 将 StateMachine 的 Json 输出以 Graphviz 文本输出
std::string StateMachineJsonToGraphviz(const Json &js);

}
}

#endif //TBOX_FLOW_JSON_TO_GRAPHVIZ_H_20230323
