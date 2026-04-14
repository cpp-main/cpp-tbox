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

from data_processing import DataProcessing
from data_source import DataSource
from transform.zoomable import ZoomableGraphicsView
from transform.zoomable_svg import ZoomableSvgGraphicsView

from datetime import datetime
from PyQt5.QtWidgets import (QMainWindow, QLabel, QVBoxLayout, QWidget, QPushButton, QHBoxLayout)
from PyQt5.QtCore import Qt, QTimer
from PyQt5.QtGui import QFont

class GraphvizViewer(QMainWindow):
    def __init__(self, pipe_name):
        super().__init__()
        self.pipe_name = pipe_name
        self.current_pixmap = None
        self.current_svg_bytes = b''
        self._use_svg = False
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

        # Top bar: timestamp label + solution toggle button
        top_bar = QWidget()
        top_bar.setFixedHeight(24)
        top_bar_layout = QHBoxLayout(top_bar)
        top_bar_layout.setContentsMargins(0, 0, 4, 0)
        top_bar_layout.setSpacing(4)

        self.timestamp_label = QLabel()
        self.timestamp_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.timestamp_label.setStyleSheet("background-color: #f0f0f0; padding: 2px;")
        self.timestamp_label.setFont(QFont("Arial", 8))

        self.toggle_btn = QPushButton("高清缩放: 关")
        self.toggle_btn.setFixedWidth(100)
        self.toggle_btn.setFixedHeight(20)
        self.toggle_btn.setFont(QFont("Arial", 8))
        self.toggle_btn.setCheckable(True)
        self.toggle_btn.clicked.connect(self._toggle_solution)

        top_bar_layout.addWidget(self.timestamp_label, stretch=1)
        top_bar_layout.addWidget(self.toggle_btn)

        # Create zoomable image label (Solution A by default)
        self.image_label = ZoomableGraphicsView()

        # Add widgets to main layout
        layout.addWidget(top_bar)
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

    def _toggle_solution(self):
        """切换渲染方案：方案A（PNG直接缩放）↔ 方案B（SVG高清缩放）"""
        self._use_svg = not self._use_svg

        if self._use_svg:
            new_view = ZoomableSvgGraphicsView()
            self.toggle_btn.setText("高清缩放: 开")
        else:
            new_view = ZoomableGraphicsView()
            self.toggle_btn.setText("高清缩放: 关")

        old_view = self.image_label

        # 让旧视图的后台线程自动放弃待处理结果
        if hasattr(old_view, '_render_gen'):
            old_view._render_gen += 1
        if hasattr(old_view, '_sharpen_timer'):
            old_view._sharpen_timer.stop()

        layout = self.centralWidget().layout()
        layout.replaceWidget(old_view, new_view)
        old_view.hide()
        old_view.deleteLater()
        self.image_label = new_view

        # 将当前图像立即重放到新视图
        if self.current_pixmap is not None:
            self.image_label.update_image(self.current_pixmap, self.current_svg_bytes)

    def update_graph(self, pixmap, svg_bytes=b''):
        if pixmap != self.current_pixmap:
            self.current_pixmap = pixmap
            self.current_svg_bytes = svg_bytes
            self.image_label.update_image(self.current_pixmap, self.current_svg_bytes)

    def updateTime(self):
        # Update timestamp
        current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.timestamp_label.setText(current_time)

    def receive_graph_data(self, data):
        self.updateTime()
        self.data_processing.receive_data(data)

    def update_image(self):
        if self.current_pixmap is None:
            return
        self.image_label.update_image(self.current_pixmap, self.current_svg_bytes)

    def closeEvent(self, event):
        self.data_source.stop()
        self.data_processing.stop()
        self.data_source.wait()
        self.data_processing.wait()
        if os.path.exists(self.pipe_name):
            os.remove(self.pipe_name)
        event.accept()
