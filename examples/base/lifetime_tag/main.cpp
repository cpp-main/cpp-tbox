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
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include <tbox/base/lifetime_tag.hpp>
#include <iostream>

using namespace std;

int main()
{
    struct Tmp {
        int value;
        tbox::LifetimeTag lifetime_tag;
    };

    auto tmp = new Tmp{10};
    auto watch = tmp->lifetime_tag.get();
    auto print_func = [tmp, watch] (const char *prompt) {
        if (watch)
            cout << '[' << prompt << "] it's alive, value " << tmp->value << endl;
        else
            cout << '[' << prompt << "] it's not alive" << endl;
    };

    print_func("A");    //! it's alive, value 10
    delete tmp;

    print_func("B");    //! it's not alive

    return 0;
}
