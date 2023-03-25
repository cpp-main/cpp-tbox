#ifndef TBOX_FLOW_JSON_TO_GRAPHVIZ_H_20230323
#define TBOX_FLOW_JSON_TO_GRAPHVIZ_H_20230323

#include <tbox/base/json_fwd.h>

namespace tbox {
namespace flow {

std::string ActionJsonToGraphviz(const Json &js);

std::string StateMachineJsonToGraphviz(const Json &js);

}
}

#endif //TBOX_FLOW_JSON_TO_GRAPHVIZ_H_20230323
