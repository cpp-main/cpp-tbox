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

#include <tbox/base/log.h>
#include <tbox/base/log_output.h>
#include <tbox/util/fs.h>
#include <tbox/util/string.h>
#include <tbox/util/timestamp.h>

#include "sink.h"

namespace tbox {
namespace trace {

TEST(Sink, Base) {
  std::string path_prefix = "/tmp/cpp-tbox-test/trace-sink";

  auto &ts = Sink::GetInstance();
  ts.setPathPrefix(path_prefix);

  ts.enable();
  ts.commitRecord("void hello()", "a", 1, 100, 10);
  ts.commitRecord("int world(int, std::string)", "a", 2, 101, 1);
  ts.commitRecord("void hello()", "b", 1, 200, 10);
  ts.commitRecord("bool bye(double)", "b", 3, 201, 100);
  ts.disable();

  std::string path = ts.getDirPath();
  std::string name_list_filename = path + "/names.txt";
  std::string thread_list_filename = path + "/threads.txt";
  std::string module_list_filename = path + "/modules.txt";
  std::string record_filename = ts.getCurrRecordFilename();

  ASSERT_TRUE(util::fs::IsFileExist(name_list_filename));
  ASSERT_TRUE(util::fs::IsFileExist(thread_list_filename));
  ASSERT_TRUE(util::fs::IsFileExist(module_list_filename));
  ASSERT_TRUE(util::fs::IsFileExist(record_filename));

  {
    std::string name_list_content;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(name_list_filename, name_list_content));
    std::string target_content = "void hello() at L1\nint world(int, std::string) at L2\nbool bye(double) at L3\n";
    EXPECT_EQ(name_list_content, target_content);
  }

  {
    std::string module_list_content;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(module_list_filename, module_list_content));
    std::string target_content = "a\nb\n";
    EXPECT_EQ(module_list_content, target_content);
  }

  {
    std::string thread_list_content;
    ASSERT_TRUE(util::fs::ReadStringFromTextFile(thread_list_filename, thread_list_content));
    std::string target_content = std::to_string(::syscall(SYS_gettid)) + "\n";
    EXPECT_EQ(thread_list_content, target_content);
  }

  {
    std::string first_record_content;
    ASSERT_TRUE(util::fs::ReadBinaryFromFile(record_filename, first_record_content));
    auto first_record_content_hex = util::string::RawDataToHexStr(first_record_content.data(), first_record_content.size());
    std::string target_content = "64 0a 00 00 00 01 01 00 01 00 63 0a 00 00 01 01 64 00 02 01";
    EXPECT_EQ(first_record_content_hex, target_content);
  }

  util::fs::RemoveDirectory("/tmp/cpp-tbox-test");
}

TEST(Sink, MultiThread) {
  std::string path_prefix = "/tmp/cpp-tbox-test/trace-sink";

  auto &ts = Sink::GetInstance();
  ts.setPathPrefix(path_prefix);
  ts.enable();

  auto test_func = [&ts] (const std::string name) {
    for (int i = 0; i < 1000; ++i) {
      ts.commitRecord(name.c_str(), "A", 100, util::GetUtcMicroseconds(), 10);
    }
  };

  auto t = std::thread(std::bind(test_func, "sub_thread()"));
  test_func("main_thread()");

  t.join();
  ts.disable();

  //!TODO: 检查记录和条数是否有2000条
  util::fs::RemoveDirectory("/tmp/cpp-tbox-test");
}

}
}
