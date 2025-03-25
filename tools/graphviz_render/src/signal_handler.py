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
# Copyright (c) 2025 Hevake and contributors, all rights reserved.
#
# This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
# Use of this source code is governed by MIT license that can be found
# in the LICENSE file in the root of the source tree. All contributing
# project authors may be found in the CONTRIBUTORS.md file in the root
# of the source tree.
#

import signal
from PyQt5.QtCore import QObject, pyqtSignal, QTimer

class SignalHandler(QObject):
    sig_interrupt = pyqtSignal()

    def __init__(self):
        super().__init__()
        # 注册系统信号到Qt信号转发
        signal.signal(signal.SIGINT, self.handle_signal)

    def handle_signal(self, signum, _):
        print(f"捕获到信号 {signum}")
        self.sig_interrupt.emit()  # 通过Qt信号触发主线程退出
