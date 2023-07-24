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
#ifndef TBOX_MAIN_EXAMPLE_APP1_H_20211226
#define TBOX_MAIN_EXAMPLE_APP1_H_20211226

#include <tbox/main/main.h>

namespace app1 {

class App : public tbox::main::Module
{
  public:
    App(tbox::main::Context &ctx);
    ~App();
};

}

#endif //TBOX_MAIN_EXAMPLE_APP1_H_20211226
