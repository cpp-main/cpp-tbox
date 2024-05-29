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

#include <sys/syscall.h>
#include <thread>

#include <tbox/util/fs.h>
#include <tbox/util/string.h>
#include <tbox/util/timestamp.h>

#include "sink.h"

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

TEST(Sink, Base) {
  std::string path_prefix = "/tmp/cpp-tbox-test/trace-sink";
  std::string pid_str = std::to_string(::getpid());
  std::string time_str = GetTimeStr();

  auto &ts = Sink::GetInstance();
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
  ASSERT_TRUE(util::fs::IsFileExist(name_list_filename));
  ASSERT_TRUE(util::fs::IsFileExist(thread_list_filename));
  ASSERT_TRUE(util::fs::IsFileExist(record_filename));

  {
    std::string name_list_content;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(name_list_filename, name_list_content));
    std::string target_content = "void hello()\r\nint world(int, std::string)\r\nbool bye(double)\r\n";
    EXPECT_EQ(name_list_content, target_content);
  }

  {
    std::string thread_list_content;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(thread_list_filename, thread_list_content));
    std::string target_content = std::to_string(::syscall(SYS_gettid)) + "\r\n";
    EXPECT_EQ(thread_list_content, target_content);
  }

  {
    std::string first_record_content;
    ASSERT_TRUE(util::fs::ReadBinaryFromFile(record_filename, first_record_content));
    auto first_record_content_hex = util::string::RawDataToHexStr(first_record_content.data(), first_record_content.size());
    std::string target_content = "64 0a 00 00 01 01 00 01 63 0a 00 00";
    EXPECT_EQ(first_record_content_hex, target_content);
  }

  util::fs::RemoveDirectory("/tmp/cpp-tbox-test");
}

TEST(Sink, MultiThread) {
  std::string path_prefix = "/tmp/cpp-tbox-test/trace-sink";
  std::string pid_str = std::to_string(::getpid());
  std::string time_str = GetTimeStr();

  auto &ts = Sink::GetInstance();
  ts.setPathPrefix(path_prefix);
  ts.enable();

  auto test_func = [&ts] (const std::string name) {
    for (int i = 0; i < 1000; ++i) {
      ts.commitRecord(name.c_str(), util::GetCurrentMicrosecondsFrom1970(), 10);
    }
  };

  auto t = std::thread(std::bind(test_func, "sub_thread()"));
  test_func("main_thread()");

  t.join();
  ts.disable();

  util::fs::RemoveDirectory("/tmp/cpp-tbox-test");
}

}
}
