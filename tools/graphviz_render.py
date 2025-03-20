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
import asyncio
import subprocess
from datetime import datetime
from io import BytesIO
from PIL import Image
from PyQt6.QtWidgets import (QApplication, QMainWindow, QLabel, QVBoxLayout,
                           QWidget, QScrollArea, QFrame)
from PyQt6.QtCore import Qt, QTimer, pyqtSignal, QThread, QSize, QPoint
from PyQt6.QtGui import QImage, QPixmap, QFont, QPainter, QMouseEvent, QWheelEvent

class ZoomableLabel(QLabel):
    def __init__(self, parent=None):
        super().__init__(parent)
        self.zoom_factor = 1.0
        self.min_zoom = 0.1
        self.max_zoom = 10.0
        self.zoom_step = 1.2
        self.panning = False
        self.last_pos = None
        self.offset = QPoint(0, 0)
        self.current_pixmap = None
        self.setMouseTracking(True)
        self.initial_fit = True
        self.last_click_time = 0
        self.last_click_pos = None

    def wheelEvent(self, event: QWheelEvent):
        if self.current_pixmap is None:
            return

        # Get cursor position relative to the widget
        cursor_pos = event.position()

        # Update zoom factor
        if event.angleDelta().y() > 0:
            self.zoom_at_position(cursor_pos.toPoint(), self.zoom_step)
        else:
            self.zoom_at_position(cursor_pos.toPoint(), 1.0 / self.zoom_step)

    def mousePressEvent(self, event: QMouseEvent):
        if event.button() == Qt.MouseButton.RightButton:
            self.panning = True
            self.last_pos = event.position().toPoint()
            self.setCursor(Qt.CursorShape.ClosedHandCursor)
        elif event.button() == Qt.MouseButton.MiddleButton:
            # Reset zoom and position when middle mouse button is pressed
            self.fit_to_window()
        elif event.button() == Qt.MouseButton.LeftButton:
            current_time = event.timestamp()
            current_pos = event.position().toPoint()

            # Check if this is a double click (within 500ms and same position)
            if (current_time - self.last_click_time < 500 and
                self.last_click_pos is not None and
                (current_pos - self.last_click_pos).manhattanLength() < 5):
                # Double click detected - zoom in at cursor position
                self.zoom_at_position(current_pos, self.zoom_step)
                self.last_click_time = 0  # Reset click time
                self.last_click_pos = None  # Reset click position
            else:
                # Single click - start panning
                self.panning = True
                self.last_pos = current_pos
                self.setCursor(Qt.CursorShape.ClosedHandCursor)
                # Update last click info for potential double click
                self.last_click_time = current_time
                self.last_click_pos = current_pos

    def mouseReleaseEvent(self, event: QMouseEvent):
        if event.button() == Qt.MouseButton.RightButton or event.button() == Qt.MouseButton.LeftButton:
            self.panning = False
            self.setCursor(Qt.CursorShape.ArrowCursor)

    def mouseMoveEvent(self, event: QMouseEvent):
        if self.panning and self.last_pos is not None:
            delta = event.position().toPoint() - self.last_pos
            self.offset += delta
            self.last_pos = event.position().toPoint()
            self.update_image(self.current_pixmap)

    def update_image(self, pixmap: QPixmap, zoom_center: QPoint = None):
        if pixmap is None:
            return

        self.current_pixmap = pixmap

        # If this is the first image, fit it to the window
        if self.initial_fit:
            self.initial_fit = False
            self.fit_to_window()
            return

        # Calculate new size based on zoom factor
        new_width = int(pixmap.width() * self.zoom_factor)
        new_height = int(pixmap.height() * self.zoom_factor)

        # Scale the pixmap
        scaled_pixmap = pixmap.scaled(
            new_width,
            new_height,
            Qt.AspectRatioMode.KeepAspectRatio,
            Qt.TransformationMode.SmoothTransformation
        )

        # Create a new pixmap with the same size as the label
        result_pixmap = QPixmap(self.size())
        result_pixmap.fill(Qt.GlobalColor.transparent)

        # Create painter for the result pixmap
        painter = QPainter(result_pixmap)

        # Calculate the position to draw the scaled image
        x = (self.width() - scaled_pixmap.width()) // 2 + self.offset.x()
        y = (self.height() - scaled_pixmap.height()) // 2 + self.offset.y()

        # Draw the scaled image
        painter.drawPixmap(x, y, scaled_pixmap)
        painter.end()

        # Update the display
        super().setPixmap(result_pixmap)

        # Update minimum size
        self.setMinimumSize(new_width, new_height)

    def fit_to_window(self):
        if self.current_pixmap is None:
            return

        # Calculate scaling ratio to fit the window
        width_ratio = self.width() / self.current_pixmap.width()
        height_ratio = self.height() / self.current_pixmap.height()
        self.zoom_factor = min(width_ratio, height_ratio)

        # Reset offset
        self.offset = QPoint(0, 0)

        # Update display
        self.update_image(self.current_pixmap)

    def resizeEvent(self, event):
        super().resizeEvent(event)
        # Always fit to window on resize
        if self.current_pixmap is not None:
            self.fit_to_window()

    def zoom_at_position(self, pos: QPoint, factor: float):
        if self.current_pixmap is None:
            return

        # Calculate cursor position relative to the image
        image_x = pos.x() - (self.width() - self.current_pixmap.width() * self.zoom_factor) / 2 - self.offset.x()
        image_y = pos.y() - (self.height() - self.current_pixmap.height() * self.zoom_factor) / 2 - self.offset.y()

        # Calculate the ratio of cursor position to image size
        ratio_x = image_x / (self.current_pixmap.width() * self.zoom_factor)
        ratio_y = image_y / (self.current_pixmap.height() * self.zoom_factor)

        # Update zoom factor
        old_zoom = self.zoom_factor
        new_zoom = min(self.zoom_factor * factor, self.max_zoom)

        # Calculate new image size
        new_width = int(self.current_pixmap.width() * new_zoom)
        new_height = int(self.current_pixmap.height() * new_zoom)

        # Calculate new cursor position relative to the image
        new_image_x = ratio_x * new_width
        new_image_y = ratio_y * new_height

        # Calculate new offset to keep cursor position fixed
        new_x = pos.x() - (self.width() - new_width) / 2 - new_image_x
        new_y = pos.y() - (self.height() - new_height) / 2 - new_image_y

        # Update zoom and offset
        self.zoom_factor = new_zoom
        self.offset = QPoint(int(new_x), int(new_y))

        # Update display
        self.update_image(self.current_pixmap)

class PipeReader(QThread):
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
                        # Small sleep to prevent CPU overuse
                        self.msleep(10)
            except Exception as e:
                print(f"Error reading from pipe: {e}")
                self.msleep(1000)  # Wait before retrying

    def stop(self):
        self.running = False

class GraphvizViewer(QMainWindow):
    def __init__(self, pipe_name):
        super().__init__()
        self.pipe_name = pipe_name
        self.original_image = None
        self.current_pixmap = None
        self.last_dot_data = ''
        self.setup_ui()

    def setup_ui(self):
        self.setWindowTitle(f"Graphviz: {self.pipe_name}")
        self.setMinimumSize(300, 200)

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
        self.image_label = ZoomableLabel()
        self.image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.image_label.setMinimumSize(1, 1)  # Allow shrinking

        # Add widgets to main layout
        layout.addWidget(self.timestamp_label)
        layout.addWidget(self.image_label)

        # Setup pipe reader
        self.pipe_reader = PipeReader(self.pipe_name)
        self.pipe_reader.data_received.connect(self.update_graph)
        self.pipe_reader.start()

        # Setup resize timer for debouncing
        self.resize_timer = QTimer()
        self.resize_timer.setSingleShot(True)
        self.resize_timer.timeout.connect(self.update_image)

        # Connect resize event
        self.resizeEvent = self.on_resize

    def on_resize(self, event):
        super().resizeEvent(event)
        # Debounce resize events
        self.resize_timer.start(100)

    def update_graph(self, dot_data):
        if dot_data != self.last_dot_data:
            self.last_dot_data = dot_data

            try:
                # Render DOT data using Graphviz
                proc = subprocess.Popen(
                    ['dot', '-Tpng'],
                    stdin=subprocess.PIPE,
                    stdout=subprocess.PIPE
                )
                output, _ = proc.communicate(dot_data.encode())

                # Convert to PIL Image
                self.original_image = Image.open(BytesIO(output))

                # Update image display
                self.update_image()

            except Exception as e:
                print(f"Error rendering graph: {e}")

        # Update timestamp
        current_time = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.timestamp_label.setText(current_time)

    def update_image(self):
        if self.original_image is None:
            return

        try:
            # Convert PIL image to QImage directly
            if self.original_image.mode == 'RGBA':
                # For RGBA images, use direct conversion
                qimg = QImage(
                    self.original_image.tobytes(),
                    self.original_image.width,
                    self.original_image.height,
                    self.original_image.width * 4,
                    QImage.Format.Format_RGBA8888
                )
            else:
                # For other formats, convert to RGBA first
                rgba_image = self.original_image.convert('RGBA')
                qimg = QImage(
                    rgba_image.tobytes(),
                    rgba_image.width,
                    rgba_image.height,
                    rgba_image.width * 4,
                    QImage.Format.Format_RGBA8888
                )

            # Create pixmap from QImage
            pixmap = QPixmap.fromImage(qimg)

            # Update the zoomable label
            self.image_label.update_image(pixmap)

        except Exception as e:
            print(f"Error updating image: {e}")

    def closeEvent(self, event):
        self.pipe_reader.stop()
        self.pipe_reader.wait()
        if os.path.exists(self.pipe_name):
            os.remove(self.pipe_name)
        event.accept()

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
    viewer.show()
    sys.exit(app.exec())

    os.remove(pipe_name)

if __name__ == "__main__":
    main()
