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

#ifndef TBOX_FLOW_ACTION_REASON_H_20240317
#define TBOX_FLOW_ACTION_REASON_H_20240317

#define ACTION_REASON_NONE              0
#define ACTION_REASON_ACTION_TIMEOUT    1   //!< "ActionTimeout"
#define ACTION_REASON_FUNCTION_ACTION   2   //!< "FunctionAction"
#define ACTION_REASON_SLEEP_ACTION      3   //!< "SleepAction"
#define ACTION_REASON_SUCC_ACTION       4   //!< "SuccAction"
#define ACTION_REASON_FAIL_ACTION       5   //!< "FailAction"
#define ACTION_REASON_START_CHILD_FAIL  6   //!< "StartChildFail"
#define ACTION_REASON_REPEAT_NO_TIMES   7   //!< "RepeatNoTimes"
#define ACTION_REASON_SWITCH_FAIL       8   //!< "SwitchFail"
#define ACTION_REASON_SWITCH_SKIP       9   //!< "SwitchSkip"
#define ACTION_REASON_IF_THEN_SKIP     10   //!< "IfThenSkip"

//! 保存 1000 以内供 Action 自用，使用者自定义的 Reason 需 >= 1000

#endif //TBOX_FLOW_ACTION_REASON_H_20240317
