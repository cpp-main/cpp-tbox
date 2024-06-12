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
#ifndef TBOX_TRACE_PARSER_WRITER_H_20240530
#define TBOX_TRACE_PARSER_WRITER_H_20240530

#include <cstdint>
#include <string>
#include <fstream>

namespace tbox {
namespace trace {

class Writer {
 public:
  bool open(const std::string &filename);

  bool writeHeader();
  bool writeRecorder(const std::string &name, const std::string &module,
                     const std::string &tid, uint64_t start_ts_us, uint64_t duration_us);
  bool writeFooter();

 private:
  std::ofstream ofs_;
  bool is_first_record_ = true;
};

}
}

#endif //TBOX_TRACE_PARSER_WRITER_H_20240530
