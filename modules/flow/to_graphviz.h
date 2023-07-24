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
#ifndef TBOX_FLOW_TO_GRAPHVIZ_H_20230323
#define TBOX_FLOW_TO_GRAPHVIZ_H_20230323

#include <tbox/base/json_fwd.h>

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
