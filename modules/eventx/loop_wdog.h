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
#ifndef TBOX_LOOP_WDOG_H_20221110
#define TBOX_LOOP_WDOG_H_20221110

#include <string>
#include <tbox/event/loop.h>

namespace tbox {
namespace eventx {

/**
 * Loop看门狗（Loop阻塞监控器）
 *
 * 正常情况下，Loop线程是不可以执行有阻塞性的动作的。如果Loop发生了阻塞，
 * 希望能被立即暴露出来。LoopWDog就是实现该功能而设计的。
 * 它每5秒让已注册的Loop执行心跳操作，然后等待1秒去检查这些Loop是否已经执行了心跳操作。
 * 如果Loop所在线程发生了阻塞，Loop所在的线程不能及时地执行心跳动作，从而可以判定Loop
 * 所在的线程是否已发生阻塞，达到对Loop进行监控的目的
 */
class LoopWDog {
  public:
    using LoopBlockCallback = std::function<void (const std::string&)>;

    //! 在 main() 中调用
    static void Start();    //! 启动线程监护
    static void Stop();     //! 停止线程监护

    //! (可选) 设置Loop阻塞时的回调
    static void SetLoopBlockCallback(const LoopBlockCallback &cb);

    //! 注册与删除要被监控的Loop
    static void Register(event::Loop *loop, const std::string &loop_name);
    static void Unregister(event::Loop *loop);
};

}
}

#endif //TBOX_LOOP_WDOG_H_20221110
