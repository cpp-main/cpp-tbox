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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "stdio.h"
#include "../impl/service/stdio.h"
#include <tbox/base/assert.h>

namespace tbox {
namespace terminal {

Stdio::Stdio(event::Loop *wp_loop, TerminalInteract *wp_terminal) :
    impl_(new Impl(wp_loop, wp_terminal))
{
    TBOX_ASSERT(impl_ != nullptr);
}

Stdio::~Stdio()
{
    delete impl_;
}

bool Stdio::initialize()
{
    return impl_->initialize();
}

void Stdio::stop()
{
    return impl_->stop();
}

bool Stdio::start()
{
    return impl_->start();
}

void Stdio::cleanup()
{
    return impl_->cleanup();
}

}
}
