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
 * Copyright (c) 2023 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_LOG_SYNC_STDOUT_SINK_H_20231006
#define TBOX_LOG_SYNC_STDOUT_SINK_H_20231006

#include "sink.h"

namespace tbox {
namespace log {

class SyncStdoutSink : public Sink {
  protected:
    virtual void onLogFrontEnd(const LogContent *content) override;
};

}
}

#endif //TBOX_LOG_SYNC_STDOUT_SINK_H_20231006
