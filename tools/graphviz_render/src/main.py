#!/usr/bin/env python3

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

import os
import sys
import signal

from ui.viewer import GraphvizViewer
from signal_handler import SignalHandler
from PyQt5.QtWidgets import (QApplication)

def create_pipe(pipe_name):
    if os.path.exists(pipe_name) :
        os.remove(pipe_name)
    os.mkfifo(pipe_name)

def print_usage(proc_name):
    print("This is a real-time rendering tool for Graphviz graphics.")
    print("Author: Hevake Lee\n")
    print(f"Usage: {proc_name} /some/where/you_pipe_file\n")

def main():
    if '-h' in sys.argv or '--help' in sys.argv:
        print_usage(sys.argv[0])
        sys.exit(0)

    if len(sys.argv) != 2:
        print_usage(sys.argv[0])
        sys.exit(1)

    pipe_name = sys.argv[1]
    create_pipe(pipe_name)

    app = QApplication(sys.argv)
    viewer = GraphvizViewer(pipe_name)
    handler = SignalHandler()
    handler.sig_interrupt.connect(app.quit)
    viewer.show()
    sys.exit(app.exec())

    os.remove(pipe_name)

if __name__ == "__main__":
    main()
