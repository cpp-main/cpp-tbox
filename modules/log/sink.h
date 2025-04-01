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
#ifndef TBOX_LOG_SINK_H_20220406
#define TBOX_LOG_SINK_H_20220406

#include <map>
#include <string>
#include <mutex>
#include <tbox/base/log.h>
#include <tbox/base/log_impl.h>

#define TIMESTAMP_STRING_SIZE 20

namespace tbox {
namespace log {

//! 日志打印通道类
class Sink {
  public:
    virtual ~Sink();

    void setLevel(int level);
    void setLevel(const std::string &module, int level);
    void unsetLevel(const std::string &module);

    void enableColor(bool enable);

    bool enable();
    void disable();

  protected:
    virtual void onEnable() { }
    virtual void onDisable() { }

    virtual void onLogFrontEnd(const LogContent *content) = 0;

    void handleLog(const LogContent *content);

    static void HandleLog(const LogContent *content, void *ptr);
    bool filter(int level, const std::string &module);

    void updateTimestampStr(uint32_t sec);

  protected:
    bool enable_color_ = false;
    char timestamp_str_[TIMESTAMP_STRING_SIZE]; //!2022-04-12 14:33:30

  private:
    std::mutex lock_;

    uint32_t output_id_ = 0;
    std::map<std::string, int> modules_level_;
    int default_level_ = LOG_LEVEL_MAX;

    uint32_t timestamp_sec_ = 0;
};

}
}

#endif //TBOX_LOG_SINK_H_20220406
