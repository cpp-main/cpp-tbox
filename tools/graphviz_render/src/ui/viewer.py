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
import subprocess

from io import BytesIO
from PIL import Image
from data_processing import DataProcessing
from data_source import DataSource
from transform.zoomable import ZoomableGraphicsView


from PIL import Image
from datetime import datetime
from PyQt5.QtWidgets import (QMainWindow, QLabel, QVBoxLayout,QWidget)
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QImage, QPixmap, QFont

class GraphvizViewer(QMainWindow):
    def __init__(self, pipe_name):
        super().__init__()
        self.pipe_name = pipe_name
        self.current_pixmap = None
        self.setup_ui()

    def setup_ui(self):
        self.setWindowTitle(f"Graphviz: {self.pipe_name}")
        self.showMaximized()

        # Create central widget and layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        layout = QVBoxLayout(central_widget)
        layout.setContentsMargins(0, 0, 0, 0)  # Remove margins
        layout.setSpacing(0)  # Remove spacing

        # Create timestamp label with minimal height
        self.timestamp_label = QLabel()
        self.timestamp_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.timestamp_label.setStyleSheet("background-color: #f0f0f0; padding: 2px;")
        self.timestamp_label.setFont(QFont("Arial", 8))  # Smaller font
        self.timestamp_label.setFixedHeight(20)  # Fixed height for timestamp

        # Create zoomable image label
        self.image_label = ZoomableGraphicsView()
        # self.image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        # self.image_label.setMinimumSize(1, 1)  # Allow shrinking

        # Add widgets to main layout
        layout.addWidget(self.timestamp_label)
        layout.addWidget(self.image_label)

        self.data_processing = DataProcessing()
        self.data_processing.data_received.connect(self.update_graph)
        self.data_processing.start()

        # Setup pipe reader
        self.data_source = DataSource(self.pipe_name)
        self.data_source.data_received.connect(self.receive_graph_data)
        self.data_source.start()

        # Setup resize timer for debouncing
        self.resize_timer = QTimer()
        self.resize_timer.setSingleShot(True)
        self.resize_timer.timeout.connect(self.update_image)

        # Connect resize event
        self.resizeEvent = self.on_resize

    def on_resize(self, event):
        super().resizeEvent(event)
        # Debounce resize events
        # self.resize_timer.start(100)

    def update_graph(self, pixmap):
        if pixmap != self.current_pixmap:
            self.current_pixmap = pixmap
            self.image_label.update_image(self.current_pixmap)

    def updateTime(self):
        # Update timestamp
        current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.timestamp_label.setText(current_time)
    
    def receive_graph_data(self, data):
        self.updateTime()
        self.data_processing.receive_data(data)

    def update_image(self):
        if self.update_image is None:
            return
        self.image_label.update_image(self.current_pixmap)

    def closeEvent(self, event):
        self.data_source.stop()
        self.data_processing.stop()
        self.data_source.wait()
        self.data_processing.wait()
        if os.path.exists(self.pipe_name):
            os.remove(self.pipe_name)
        event.accept()

