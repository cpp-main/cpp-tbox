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

from PyQt5.QtCore import QThread, pyqtSignal

exit_str = "pipe_read_exit"

class DataSource(QThread):
    data_received = pyqtSignal(str)

    def __init__(self, pipe_name):
        super().__init__()
        self.pipe_name = pipe_name
        self.running = True

    def run(self):
        while self.running:
            try:
                with open(self.pipe_name, 'r') as pipe:
                    while self.running:
                        data = pipe.read()
                        if data:
                            self.data_received.emit(data)
                        self.msleep(10)

            except Exception as e:
                print(f"Error reading from pipe: {e}")
                self.msleep(1000)

    def stop(self):
        self.running = False
        with open(self.pipe_name, 'w') as pipe:
            pipe.write(exit_str)
