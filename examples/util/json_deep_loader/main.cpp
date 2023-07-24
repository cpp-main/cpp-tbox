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
#include <iostream>

#include <tbox/base/json.hpp>
#include <tbox/util/json_deep_loader.h>

using namespace std;

void PrintUsage(const char *proc) {
    cout << "Usage: " << proc << " your.json" << endl;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        PrintUsage(argv[0]);
        return 0;
    }

    const char *filename = argv[1];

    try {
        auto json = tbox::util::json::LoadDeeply(filename);
        cout << json.dump(2) << endl;
    } catch (const std::exception &e) {
        cerr << "Catch: " << e.what() << endl;
    }
    return 0;
}
