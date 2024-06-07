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
 * Copyright (c) 2024 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_MAIN_TRACE_H_20240607
#define TBOX_MAIN_TRACE_H_20240607

#include <tbox/base/json_fwd.h>
#include "context.h"

namespace tbox {
namespace main {

class Trace {
  public:
    void fillDefaultConfig(Json &cfg) const;
    bool initialize(Context &ctx, const Json &cfg);

  protected:
    void initShell(terminal::TerminalNodes &term);
};

}
}

#endif //TBOX_MAIN_TRACE_H_20240607
