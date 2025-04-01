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

from PyQt5.QtWidgets import QLabel, QApplication, QOpenGLWidget
from PyQt5.QtCore import Qt, QPoint, QTime, QTimer
from PyQt5.QtGui import QPixmap, QPainter, QMouseEvent, QWheelEvent, QImage

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
        self.zoom_cache = {}
        self.current_cache_key = None
        self.redraw_timer = QTimer(self)
        self.redraw_timer.setSingleShot(True)
        self.redraw_timer.timeout.connect(self.force_redraw)
        self.redraw_pending = False
        self.zoom_timer = QTimer(self)
        self.zoom_timer.setSingleShot(True)
        self.zoom_timer.timeout.connect(self.high_quality_redraw)

    def wheelEvent(self, event: QWheelEvent):
        if self.current_pixmap is None:
            return
        cursor_pos = event.pos()
        if event.angleDelta().y() > 0:
            self.zoom_at_position(cursor_pos, self.zoom_step)
        else:
            self.zoom_at_position(cursor_pos, 1.0 / self.zoom_step)
        self.zoom_timer.start(500)  # 延迟高质量重绘

    def update_image(self, pixmap: QPixmap, use_fast=False):
        if pixmap is None:
            return
        cache_key = (round(self.zoom_factor, 2), pixmap.size().width(), pixmap.size().height(), use_fast)
        if cache_key in self.zoom_cache:
            scaled_pixmap = self.zoom_cache[cache_key]
        else:
            new_width = int(pixmap.width() * self.zoom_factor)
            new_height = int(pixmap.height() * self.zoom_factor)
            transformation = Qt.FastTransformation if use_fast else Qt.SmoothTransformation
            scaled_pixmap = pixmap.scaled(new_width, new_height, Qt.KeepAspectRatio, transformation)
            self.zoom_cache[cache_key] = scaled_pixmap
            if len(self.zoom_cache) > 10:
                self.zoom_cache.pop(next(iter(self.zoom_cache)))
        result_pixmap = QPixmap(self.size())
        result_pixmap.fill(Qt.transparent)
        painter = QPainter(result_pixmap)
        x = (self.width() - scaled_pixmap.width()) // 2 + self.offset.x()
        y = (self.height() - scaled_pixmap.height()) // 2 + self.offset.y()
        painter.drawPixmap(x, y, scaled_pixmap)
        painter.end()
        super().setPixmap(result_pixmap)

    def high_quality_redraw(self):
        if self.current_pixmap:
            self.update_image(self.current_pixmap, use_fast=False)

    def mouseMoveEvent(self, event: QMouseEvent):
        if self.panning and self.last_pos is not None:
            delta = event.pos() - self.last_pos
            self.offset += delta
            self.last_pos = event.pos()
            if not self.redraw_pending:
                self.redraw_pending = True
                self.redraw_timer.start(30)

    def force_redraw(self):
        if self.redraw_pending and self.current_pixmap:
            self.update_image(self.current_pixmap, use_fast=True)
            self.redraw_pending = False
