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
#ifndef TBOX_EVENT_SELECT_LOOP_H_20240619
#define TBOX_EVENT_SELECT_LOOP_H_20240619

#include <unordered_map>

#include "../../common_loop.h"

#include <tbox/base/object_pool.hpp>
#include "types.h"

namespace tbox {
namespace event {

class SelectLoop : public CommonLoop {
  public:
    virtual ~SelectLoop() override;

  public:
    virtual void runLoop(Mode mode) override;

    virtual FdEvent* newFdEvent(const std::string &what) override;

  public:
    SelectFdSharedData* refFdSharedData(int fd);
    void unrefFdSharedData(int fd);

  protected:
    virtual void stopLoop() override { keep_running_ = false; }
    int fillFdSets(fd_set &read_set, fd_set &write_set, fd_set &except_set);
    void removeInvalidFds();  //! 清除失效了的fd

  private:
    bool keep_running_ = true;

    std::unordered_map<int, SelectFdSharedData*> fd_data_map_;
    ObjectPool<SelectFdSharedData> fd_shared_data_pool_{64};
};

}
}

#endif //TBOX_EVENT_SELECT_LOOP_H_20240619
