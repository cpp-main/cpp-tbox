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
#include "writer.h"

namespace tbox {
namespace trace {

bool Writer::open(const std::string &filename)
{
    ofs_.open(filename);
    return ofs_.is_open();
}

bool Writer::writeHeader()
{
    if (!ofs_.is_open())
        return false;

    ofs_ << R"({"otherData":{},"traceEvents":[)" << std::endl;
    ofs_.flush();

    is_first_record_ = true;
    return true;
}

bool Writer::writeRecorder(const std::string &name, const std::string &module,
                           const std::string &tid, uint64_t start_ts_us, uint64_t duration_us)
{
    if (!ofs_.is_open())
        return false;

    if (!is_first_record_)
        ofs_ << ',' << std::endl;
    is_first_record_ = false;

    ofs_  << R"({"name":")" << name
          << R"(","cat":")" << module
          << R"(","pid":"","tid":")"
          << tid << R"(","ts":)" << start_ts_us << ',';

    if (duration_us != 0)
        ofs_ << R"("ph":"X","dur":)" << duration_us;
    else
        ofs_ << R"("ph":"I")";

    ofs_ << "}";
    ofs_.flush();

    return true;
}

bool Writer::writeFooter()
{
    if (!ofs_.is_open())
        return false;

    ofs_ << std::endl << R"(]})" << std::endl;
    ofs_.flush();

    return true;
}

}
}
