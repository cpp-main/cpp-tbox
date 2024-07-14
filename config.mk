#
#     .============.
#    //  M A K E  / \
#   //  C++ DEV  /   \
#  //  E A S Y  /  \/ \
# ++ ----------.  \/\  .
#  \\     \     \ /\  /
#   \\     \     \   /
#    \\     \     \ /
#     -============'
#
# Copyright (c) 2018 Hevake and contributors, all rights reserved.
#
# This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
# Use of this source code is governed by MIT license that can be found
# in the LICENSE file in the root of the source tree. All contributing
# project authors may be found in the CONTRIBUTORS.md file in the root
# of the source tree.
#

# 核心模块
MODULES += base
MODULES += util
MODULES += event
MODULES += eventx
MODULES += log
MODULES += network
MODULES += terminal
MODULES += trace
MODULES += coroutine
MODULES += main
MODULES += run

## 非核心模块，请根据需要选择
MODULES += http
MODULES += mqtt
MODULES += flow
MODULES += alarm
MODULES += crypto
MODULES += dbus
MODULES += jsonrpc

## 第三方库依赖
THIRDPARTY += nlohmann

## 编译配置
CCFLAGS += -DENABLE_TRACE_RECORDER=1
