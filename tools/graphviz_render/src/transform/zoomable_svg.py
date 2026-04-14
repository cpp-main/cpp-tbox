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

import threading

from PyQt5.QtGui import QPixmap, QWheelEvent, QMouseEvent, QKeyEvent, QPainter, QImage
from PyQt5.QtCore import Qt, QPointF, QRectF, QPropertyAnimation, QTimer, QEasingCurve, QByteArray, pyqtSignal
from PyQt5.QtWidgets import QOpenGLWidget

try:
    from PyQt5.QtSvg import QSvgRenderer
    _SVG_SUPPORT = True
except ImportError:
    _SVG_SUPPORT = False
    print("Warning: python3-pyqt5.qtsvg not installed. Zoom sharpening disabled. "
          "Run: sudo apt install python3-pyqt5.qtsvg")

from .zoomable import ZoomableGraphicsView


class ZoomableSvgGraphicsView(ZoomableGraphicsView):
    """方案B：在方案A基础上增加 SVG 后台光栅化，缩放后保持高清。

    缩放/拖拽停止约 300ms 后，在后台线程将 SVG 光栅化为当前缩放级别对应的
    高分辨率 PNG，并更新到显示区域，消除放大后的锯齿。
    """

    # 用于从后台线程安全回调主线程：传递一个可调用对象，在主线程执行
    _call_in_main = pyqtSignal(object)

    def __init__(self, parent=None):
        super().__init__(parent)

        # 连接跨线程回调信号（Qt.AutoConnection 在跨线程时自动转为 QueuedConnection）
        self._call_in_main.connect(lambda f: f())

        # SVG 高清缩放状态
        self._svg_bytes = b''
        # pixmap_item 在场景中的固定逻辑尺寸（由初始 PNG 决定，sharpen 时不变）
        self._base_scene_w = 0.0
        self._base_scene_h = 0.0
        # 上次 sharpen 时的视图缩放比，用于跳过微小变化
        self._last_sharpened_scale = 0.0
        # 用户是否手动缩放过（防止 resizeEvent 重置缩放）
        self._user_has_zoomed = False
        # fit_to_view 时的绝对缩放比，用于动态计算缩放范围
        self._fit_scale = 1.0
        # 上次渲染时的视口中心（场景坐标），用于拖拽 skip 判断
        self._last_sharpened_vp_center = QPointF()
        # 后台渲染代次，用于丢弃过期结果
        self._render_gen = 0
        # 防止并发渲染：上一次渲染未完成时跳过新请求，待完成后自动补发
        self._is_rendering = False

        self._sharpen_timer = QTimer(self)
        self._sharpen_timer.setSingleShot(True)
        self._sharpen_timer.timeout.connect(self._on_zoom_settled)
        self._SHARPEN_DELAY_MS = 300

    def update_image(self, pixmap: QPixmap, svg_bytes: bytes = b''):
        """更新图像并自适应视图"""
        if svg_bytes:
            self._svg_bytes = svg_bytes
            self._base_scene_w = 0.0   # 等 _center_pixmap 用新 PNG 尺寸重置
            self._base_scene_h = 0.0
            self._last_sharpened_scale = 0.0
            self._user_has_zoomed = False
        self.pixmap_item.setPixmap(pixmap)
        self._center_pixmap(pixmap)
        if self.fisrt_refresh:
            self.fisrt_refresh = False
            self.fit_to_view()
            if self._svg_bytes and _SVG_SUPPORT:
                self._sharpen_timer.start(150)

    def _center_pixmap(self, pixmap: QPixmap):
        """居中放置图元，并记录基准场景尺寸"""
        # 每次新图数据到来时（_base_scene_w 已被重置为 0）重新记录基准尺寸
        if self._base_scene_w == 0.0:
            self._base_scene_w = float(pixmap.width())
            self._base_scene_h = float(pixmap.height())
        self.pixmap_item.setScale(1.0)
        self.pixmap_item.setPos(-self._base_scene_w / 2.0, -self._base_scene_h / 2.0)
        self.scene.setSceneRect(-1e6, -1e6, 2e6, 2e6)

    def fit_to_view(self):
        """自适应窗口显示"""
        if self.pixmap_item.pixmap().isNull():
            return
        if self._base_scene_w > 0:
            rect = QRectF(-self._base_scene_w / 2.0, -self._base_scene_h / 2.0,
                           self._base_scene_w, self._base_scene_h)
        else:
            rect = self.pixmap_item.sceneBoundingRect()
        self.fitInView(rect, Qt.KeepAspectRatio)
        self._fit_scale = self.transform().m11()
        self._update_zoom_limits()
        self._zoom_factor = 1.0
        # 只在非手动缩放状态触发清晰化，避免 resizeEvent 干扰用户缩放
        if self._svg_bytes and _SVG_SUPPORT and not self._user_has_zoomed:
            self._sharpen_timer.start(150)

    def _update_zoom_limits(self):
        """根据图像在窗口中的比例动态计算缩放范围。

        不变量：最大绝对缩放 = fit_scale × max_zoom ≤ 10 屏幕像素/场景单位
        大图 fit_scale 小 → max_zoom 大（可放大更多倍探索细节）
        小图 fit_scale 大 → max_zoom 小（放大空间本就够用）
        下限 min_zoom = 0.2：缩到 fit 的 20%，图像仍占窗口 20% 宽度。
        """
        if self._fit_scale <= 0:
            return
        self.min_zoom = 0.2
        raw_max = 10.0 / self._fit_scale
        self.max_zoom = max(3.0, min(20.0, raw_max))

    def wheelEvent(self, event: QWheelEvent):
        zoom_in = event.angleDelta().y() > 0
        factor = 1.25 if zoom_in else 0.8
        new_zoom = self._zoom_factor * factor
        if self.min_zoom <= new_zoom <= self.max_zoom:
            self.scale(factor, factor)
            self._zoom_factor = new_zoom
            self._user_has_zoomed = True
            if self._svg_bytes and _SVG_SUPPORT:
                self._sharpen_timer.start(self._SHARPEN_DELAY_MS)

    def keyPressEvent(self, event: QKeyEvent):
        new_zoom = 0.0
        factor = 0.0
        if event.modifiers() == Qt.ControlModifier:
            if event.key() == Qt.Key_Left:
                event.accept()
                factor = 0.8
                new_zoom = self._zoom_factor * factor
            elif event.key() == Qt.Key_Right:
                event.accept()
                factor = 1.25
                new_zoom = self._zoom_factor * factor
            if self.min_zoom <= new_zoom <= self.max_zoom:
                self.scale(factor, factor)
                self._zoom_factor = new_zoom
                self._user_has_zoomed = True
                if self._svg_bytes and _SVG_SUPPORT:
                    self._sharpen_timer.start(self._SHARPEN_DELAY_MS)
                return
        super().keyPressEvent(event)

    def mouseDoubleClickEvent(self, event: QMouseEvent):
        if event.button() == Qt.RightButton:
            event.accept()
            self._fit_and_center_animation()
            return
        elif event.button() == Qt.LeftButton:
            event.accept()
            factor = 1.25
            new_zoom = self._zoom_factor * factor
            if self.min_zoom <= new_zoom <= self.max_zoom:
                self.scale(factor, factor)
                self._zoom_factor = new_zoom
                self._user_has_zoomed = True
                if self._svg_bytes and _SVG_SUPPORT:
                    self._sharpen_timer.start(self._SHARPEN_DELAY_MS)
            return
        super().mouseDoubleClickEvent(event)

    def mouseMoveEvent(self, event: QMouseEvent):
        if self.dragging and self._svg_bytes and _SVG_SUPPORT:
            self._sharpen_timer.start(150)
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event: QMouseEvent):
        if event.button() in (Qt.RightButton, Qt.LeftButton):
            self.dragging = False
            self.setCursor(Qt.ArrowCursor)
            if self._svg_bytes and _SVG_SUPPORT:
                self._sharpen_timer.start(self._SHARPEN_DELAY_MS)
        super().mouseReleaseEvent(event)

    def _fit_and_center_animation(self):
        if self.pixmap_item.pixmap().isNull():
            return
        self._user_has_zoomed = False
        self.fit_to_view()
        final_center = self.pixmap_item.sceneBoundingRect().center()
        self._create_center_animation(final_center)

    def _create_center_animation(self, target_center: QPointF):
        anim_h = QPropertyAnimation(self.horizontalScrollBar(), b"value")
        anim_v = QPropertyAnimation(self.verticalScrollBar(), b"value")

        current_zoom = self._zoom_factor
        anim_zoom = QPropertyAnimation(self, b"zoom_factor")
        anim_zoom.setDuration(400)
        anim_zoom.setStartValue(current_zoom)
        anim_zoom.setEndValue(1.0)

        for anim in [anim_h, anim_v]:
            anim.setDuration(400)
            anim.setEasingCurve(QEasingCurve.OutQuad)

        view_center = self.mapToScene(self.viewport().rect().center())
        delta = target_center - view_center

        anim_h.setStartValue(self.horizontalScrollBar().value())
        anim_h.setEndValue(self.horizontalScrollBar().value() + delta.x())
        anim_v.setStartValue(self.verticalScrollBar().value())
        anim_v.setEndValue(self.verticalScrollBar().value() + delta.y())

        anim_zoom.start()
        anim_h.start()
        anim_v.start()

        if self._svg_bytes and _SVG_SUPPORT:
            anim_zoom.finished.connect(lambda: self._sharpen_timer.start(50))

        self.center_indicator.setPos(target_center)
        self.center_indicator.setVisible(True)
        QTimer.singleShot(800, lambda: self.center_indicator.setVisible(False))

    def _on_zoom_settled(self):
        """缩放停止后触发：在后台线程将整张 SVG 光栅化，完成后回到主线程更新。"""
        if not (_SVG_SUPPORT and self._svg_bytes):
            return
        if self._base_scene_w <= 0 or self._base_scene_h <= 0:
            return

        # 上一次渲染仍在进行，延迟重试，避免任务堆积
        if self._is_rendering:
            self._sharpen_timer.start(self._SHARPEN_DELAY_MS)
            return

        current_scale = self.transform().m11()

        vp_rect  = self.viewport().rect()
        vp_scene = self.mapToScene(vp_rect).boundingRect()

        # skip：缩放比变化 < 10% 且视口位移 < 10% 视口宽高
        if self._last_sharpened_scale > 0:
            scale_ok = abs(current_scale / self._last_sharpened_scale - 1.0) < 0.1
            if scale_ok:
                prev = self._last_sharpened_vp_center
                cur  = vp_scene.center()
                if (abs(cur.x() - prev.x()) < vp_scene.width()  * 0.1 and
                        abs(cur.y() - prev.y()) < vp_scene.height() * 0.1):
                    return

        full_rect = QRectF(-self._base_scene_w / 2.0, -self._base_scene_h / 2.0,
                            self._base_scene_w, self._base_scene_h)

        # 只渲染可见视口与图像的交集：target 像素数 ≤ 视口像素数，无需 cap
        clip_rect = vp_scene.intersected(full_rect)
        if clip_rect.isEmpty():
            return

        # 递增代次，后台线程完成时若代次已变则丢弃结果
        self._render_gen += 1
        gen = self._render_gen
        self._is_rendering = True

        # 捕获渲染参数（不在后台访问 self）
        svg_bytes     = self._svg_bytes
        base_w        = self._base_scene_w
        base_h        = self._base_scene_h
        full_raster_w = base_w * current_scale
        full_raster_h = base_h * current_scale
        offset_x      = (clip_rect.x() - full_rect.x()) / full_rect.width()  * full_raster_w
        offset_y      = (clip_rect.y() - full_rect.y()) / full_rect.height() * full_raster_h
        target_w      = max(1, int(clip_rect.width()  * current_scale))
        target_h      = max(1, int(clip_rect.height() * current_scale))
        clip_x        = clip_rect.x()
        clip_y        = clip_rect.y()
        vp_center     = vp_scene.center()

        def background_render():
            renderer = QSvgRenderer(QByteArray(svg_bytes))
            if not renderer.isValid():
                self._call_in_main.emit(lambda: setattr(self, '_is_rendering', False))
                return

            image = QImage(target_w, target_h, QImage.Format_ARGB32_Premultiplied)
            image.fill(Qt.transparent)
            painter = QPainter(image)
            painter.setRenderHint(QPainter.Antialiasing)
            painter.setRenderHint(QPainter.TextAntialiasing)
            painter.translate(-offset_x, -offset_y)
            renderer.render(painter, QRectF(0, 0, full_raster_w, full_raster_h))
            painter.end()

            def apply_on_main():
                self._is_rendering = False
                if self._render_gen != gen:
                    return
                pixmap = QPixmap.fromImage(image)
                self.pixmap_item.setPixmap(pixmap)
                self.pixmap_item.setPos(clip_x, clip_y)
                self.pixmap_item.setScale(1.0 / current_scale)
                self._last_sharpened_scale = current_scale
                self._last_sharpened_vp_center = vp_center

            self._call_in_main.emit(apply_on_main)

        threading.Thread(target=background_render, daemon=True).start()

    def resizeEvent(self, event):
        """窗口大小变化：未手动缩放则自适应，已手动缩放则保持"""
        if not self._user_has_zoomed:
            self.fit_to_view()
        super().resizeEvent(event)
