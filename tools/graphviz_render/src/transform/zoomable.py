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

from PyQt5.QtCore import pyqtProperty
from PyQt5.QtGui import QPixmap, QWheelEvent, QMouseEvent, QKeyEvent, QPainter, QColor
from PyQt5.QtCore import Qt, QPointF, QPropertyAnimation, QTimer, QEasingCurve
from PyQt5.QtWidgets import (QGraphicsView, QGraphicsScene, QGraphicsPixmapItem, QOpenGLWidget, QGraphicsEllipseItem)


class ZoomableGraphicsView(QGraphicsView):
    def __init__(self, parent=None):
        super().__init__(parent)
        
        # 初始化视图设置
        self._setup_view()
        self._setup_scene()
        self._setup_interaction()
        self._setup_indicators()
        
        # 初始化参数
        self._zoom_factor = 1.0
        self.min_zoom = 0.1
        self.max_zoom = 20.0
        self.dragging = False
        self.fisrt_refresh = True
        self.last_mouse_pos = QPointF()

    # 添加属性声明
    @pyqtProperty(float)
    def zoom_factor(self):
        return self._zoom_factor

    @zoom_factor.setter
    def zoom_factor(self, value):
        # 计算缩放比例差异
        ratio = value / self._zoom_factor
        self._zoom_factor = value
        # 应用缩放变换
        self.scale(ratio, ratio)

    def _setup_view(self):
        """配置视图参数"""
        self.setViewport(QOpenGLWidget())  # OpenGL加速
        self.setRenderHints(QPainter.Antialiasing | QPainter.SmoothPixmapTransform | QPainter.TextAntialiasing)
        self.setCacheMode(QGraphicsView.CacheBackground)
        self.setViewportUpdateMode(QGraphicsView.FullViewportUpdate)

    def _setup_scene(self):
        """配置场景和图像项"""
        self.scene = QGraphicsScene(self)
        self.scene.setSceneRect(-1e6, -1e6, 2e6, 2e6)  # 超大场景范围
        self.setScene(self.scene)
        
        self.pixmap_item = QGraphicsPixmapItem()
        self.pixmap_item.setTransformationMode(Qt.SmoothTransformation)
        self.pixmap_item.setShapeMode(QGraphicsPixmapItem.BoundingRectShape)
        self.scene.addItem(self.pixmap_item)

    def _setup_interaction(self):
        """配置交互参数"""
        self.setDragMode(QGraphicsView.NoDrag)
        self.setTransformationAnchor(QGraphicsView.AnchorUnderMouse)
        self.setResizeAnchor(QGraphicsView.AnchorUnderMouse)
        self.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setMouseTracking(True)

    def _setup_indicators(self):
        """配置视觉指示器"""
        # 居中指示器
        self.center_indicator = QGraphicsEllipseItem(-8, -8, 16, 16)
        self.center_indicator.setPen(QColor(255, 0, 0, 150))
        self.center_indicator.setBrush(QColor(255, 0, 0, 50))
        self.center_indicator.setZValue(100)
        self.center_indicator.setVisible(False)
        self.scene.addItem(self.center_indicator)

    def update_image(self, pixmap: QPixmap):
        """更新图像并自适应视图"""
        self.pixmap_item.setPixmap(pixmap)
        self._center_pixmap(pixmap)
        if self.fisrt_refresh:
            self.fisrt_refresh = False
            self.fit_to_view()
        
    def _center_pixmap(self, pixmap: QPixmap):
        """居中放置图元"""
        self.pixmap_item.setPos(-pixmap.width()/2, -pixmap.height()/2)
        self.scene.setSceneRect(-1e6, -1e6, 2e6, 2e6)  # 重置场景范围

    def fit_to_view(self):
        """自适应窗口显示"""
        if self.pixmap_item.pixmap().isNull():
            return

        # 自动计算场景中的图元边界
        rect = self.pixmap_item.sceneBoundingRect()
        self.fitInView(rect, Qt.KeepAspectRatio)
        self._zoom_factor = 1.0

    def wheelEvent(self, event: QWheelEvent):
        """滚轮缩放处理"""
        zoom_in = event.angleDelta().y() > 0
        factor = 1.25 if zoom_in else 0.8
        new_zoom = self._zoom_factor * factor
        
        # 应用缩放限制
        if self.min_zoom <= new_zoom <= self.max_zoom:
            self.scale(factor, factor)
            self._zoom_factor = new_zoom

    def keyPressEvent(self, event: QKeyEvent):
        """增加键盘快捷键支持"""
        new_zoom = 0.0
        factor = 0.0
        if event.modifiers() == Qt.ControlModifier:
            if event.key() == Qt.Key_Left:  # Ctrl + Left
                event.accept()
                factor = 0.8
                new_zoom = self._zoom_factor * factor

            elif event.key() == Qt.Key_Right:  # Ctrl + right
                event.accept()
                factor = 1.25
                new_zoom = self._zoom_factor * factor

            # 应用缩放限制
            if  self.min_zoom <= new_zoom <= self.max_zoom:
                self.scale(factor, factor)
                self._zoom_factor = new_zoom
                return

        super().keyPressEvent(event)

    def mousePressEvent(self, event: QMouseEvent):
        """鼠标按下事件处理"""
        # 左键拖拽
        if event.button() in (Qt.RightButton, Qt.LeftButton):
            self.dragging = True
            self.last_mouse_pos = event.pos()
            self.setCursor(Qt.ClosedHandCursor)
            
        super().mousePressEvent(event)
    
    def mouseDoubleClickEvent(self, event: QMouseEvent):
        """鼠标双击事件处理（增强版）"""
        if event.button() == Qt.RightButton:
            event.accept()
            self._fit_and_center_animation()  # 改为新的组合动画方法
            return
        elif event.button() == Qt.LeftButton:
            event.accept()
            factor = 2
            new_zoom = self._zoom_factor * factor

            # 应用缩放限制
            if self.min_zoom <= new_zoom <= self.max_zoom:
                self.scale(factor, factor)
                self._zoom_factor = new_zoom
            return

        super().mouseDoubleClickEvent(event)

    def mouseMoveEvent(self, event: QMouseEvent):
        """鼠标移动事件处理"""
        if self.dragging:
            delta = event.pos() - self.last_mouse_pos
            self.last_mouse_pos = event.pos()
            
            # 更新滚动条实现拖拽
            self.horizontalScrollBar().setValue(
                self.horizontalScrollBar().value() - delta.x())
            self.verticalScrollBar().setValue(
                self.verticalScrollBar().value() - delta.y())
            
        super().mouseMoveEvent(event)

    def mouseReleaseEvent(self, event: QMouseEvent):
        """鼠标释放事件处理"""
        if event.button() in (Qt.RightButton, Qt.LeftButton):
            self.dragging = False
            self.setCursor(Qt.ArrowCursor)
        super().mouseReleaseEvent(event)

    def _fit_and_center_animation(self):
        """自适应并居中动画组合"""
        if self.pixmap_item.pixmap().isNull():
            return

        # 先执行自适应调整
        self.fit_to_view()
        
        # 获取最终场景中心坐标
        final_center = self.pixmap_item.sceneBoundingRect().center()
        
        # 创建组合动画
        self._create_center_animation(final_center)

    def _create_center_animation(self, target_center: QPointF):
        """创建居中动画序列"""
        # 平移动画
        anim_h = QPropertyAnimation(self.horizontalScrollBar(), b"value")
        anim_v = QPropertyAnimation(self.verticalScrollBar(), b"value")
        
        # 缩放动画
        current_zoom = self._zoom_factor
        anim_zoom = QPropertyAnimation(self, b"zoom_factor")
        anim_zoom.setDuration(400)
        anim_zoom.setStartValue(current_zoom)
        anim_zoom.setEndValue(1.0)  # 自适应后的标准缩放值
        
        # 配置动画参数
        for anim in [anim_h, anim_v]:
            anim.setDuration(400)
            anim.setEasingCurve(QEasingCurve.OutQuad)

        # 计算目标滚动值
        view_center = self.mapToScene(self.viewport().rect().center())
        delta = target_center - view_center
        
        # 设置动画参数
        anim_h.setStartValue(self.horizontalScrollBar().value())
        anim_h.setEndValue(self.horizontalScrollBar().value() + delta.x())
        
        anim_v.setStartValue(self.verticalScrollBar().value())
        anim_v.setEndValue(self.verticalScrollBar().value() + delta.y())
        
        # 启动动画
        anim_zoom.start()
        anim_h.start()
        anim_v.start()
        
        # 显示指示器
        self.center_indicator.setPos(target_center)
        self.center_indicator.setVisible(True)
        QTimer.singleShot(800, lambda: self.center_indicator.setVisible(False))

    def resizeEvent(self, event):
        """窗口大小变化时保持自适应"""
        self.fit_to_view()
        super().resizeEvent(event)
