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
#include <gtest/gtest.h>
#include <tbox/util/fs.h>

#include "trace_sink.h"

namespace tbox {
namespace trace {

namespace {
std::string GetTimeStr()
{
    char timestamp[16]; //! 固定长度16B，"20220414_071023"
    time_t ts_sec = time(nullptr);
    struct tm tm;
    localtime_r(&ts_sec, &tm);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", &tm);
    return timestamp;
}
}

TEST(TrackSink, Base) {
  std::string path_prefix = "/tmp/test/cpp-tbox/trace";
  std::string pid_str = std::to_string(::getpid());
  std::string time_str = GetTimeStr();

  auto &ts = TrackSink::GetInstance();
  ts.setPathPrefix(path_prefix);
  ts.setRecordFileMaxSize(10);
  ts.enable();
  ts.commitRecord("void hello()", 100, 10);
  ts.commitRecord("int world(int, std::string)", 101, 1);
  ts.commitRecord("void hello()", 200, 10);
  ts.commitRecord("bool bye(double)", 201, 100);
  ts.disable();

  std::string path = path_prefix + '.' + pid_str;
  std::string name_list_filename = path + "/name_list.txt";
  std::string thread_list_filename = path + "/thread_list.txt";
  std::string record_filename = path + "/records/" + time_str + ".bin";
  EXPECT_TRUE(util::fs::IsFileExist(name_list_filename));
  EXPECT_TRUE(util::fs::IsFileExist(thread_list_filename));
  EXPECT_TRUE(util::fs::IsFileExist(record_filename));
  EXPECT_TRUE(util::fs::IsFileExist(record_filename + ".1"));

  util::fs::RemoveDirectory("/tmp/test/cpp-tbox");
}

}
}
