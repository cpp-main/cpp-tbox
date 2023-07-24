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
#include "sub.h"
#include <tbox/base/log.h>

namespace app1 {

Sub::Sub(tbox::main::Context &ctx) :
    Module("sub", ctx)
{
    LogTag();
}

Sub::~Sub()
{
    LogTag();
}

bool Sub::onInit(const tbox::Json &cfg)
{
    LogTag();
    return true;
}

bool Sub::onStart()
{
    LogTag();
    return true;
}

void Sub::onStop()
{
    LogTag();
}

void Sub::onCleanup()
{
    LogTag();
}

}
