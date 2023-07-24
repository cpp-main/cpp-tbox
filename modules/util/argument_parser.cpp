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
#include "argument_parser.h"

#include <tbox/base/assert.h>
#include "string.h"

namespace tbox {
namespace util {

namespace {
/**
 * \brief   pick key and value from '--key=value'
 */
bool PickKeyAndValue(const std::string &orig, std::string &key, std::string &value)
{
    auto pos = orig.find_first_of('=');
    if (pos == std::string::npos)
        return false;

    key   = orig.substr(0, pos);
    value = string::StripQuot(orig.substr(pos + 1));
    return true;
}
}

bool ArgumentParser::parse(const std::vector<std::string> &args, int start)
{
    for (size_t i = start; i < args.size(); ++i) {
        const std::string &curr = args.at(i);
        OptionValue opt_value;
        if (i != (args.size() - 1))
            opt_value.set(args.at(i + 1));

        if (curr[0] == '-') {
            if (curr[1] == '-') {   //! 匹配 --xxxx
                const std::string opt = curr.substr(2);
                std::string key, value;
                //! handle '--key=value'
                if (PickKeyAndValue(opt, key, value)) {
                    OptionValue tmp;
                    tmp.set(value);
                    if (!handler_(0, key, tmp))
                        return false;
                } else {
                    //! handle '--key value'
                    if (!handler_(0, opt, opt_value))
                        return false;
                }
            } else {    //! handle -xyz pattern
                for (size_t j = 1; j < curr.size(); ++j) {
                    char opt = curr[j];
                    if (j != (curr.size() - 1)) {
                        OptionValue tmp;
                        if (!handler_(opt, "", tmp))
                            return false;
                    } else {
                        //! only this last opt possess value
                        if (!handler_(opt, "", opt_value))
                            return false;
                    }
                }
            }
        }

        if (opt_value.isUsed())
            ++i;
    }

    return true;

}

bool ArgumentParser::parse(int argc, const char * const * const argv, int start)
{
    TBOX_ASSERT(argc >= 1);
    TBOX_ASSERT(argv != nullptr);
    TBOX_ASSERT(start >= 0);

    std::vector<std::string> args;
    for (int i = start; i < argc; ++i) {
        const char *curr = argv[i];
        if (curr == nullptr)
            return false;
        args.push_back(curr);
    }
    return parse(args, 0);
}

}
}
