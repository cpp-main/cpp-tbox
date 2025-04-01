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
import threading
from io import BytesIO

from PIL import Image
from PyQt5.QtGui import QPixmap, QImage
from PyQt5.QtCore import QThread, pyqtSignal

class DataProcessing(QThread):
    data_received = pyqtSignal(QPixmap)

    def __init__(self):
        super().__init__()
        # 共享数据保护
        self._lock = threading.Lock()
        self._cond = threading.Condition(self._lock)   # 数据变更条件变量

        # 共享状态变量
        self._last_dot_data = ''             # 最后接收的DOT数据 
        self._running = True                 # 线程运行标志
        self._original_image = None          # 原始图像缓存
        self._pending_update = False         # 更新标记

    def run(self):
        """线程主循环（条件变量优化版）"""
        while True:
            with self._cond:
                # 等待数据变更
                self._cond.wait_for(lambda: self._pending_update or (not self._running))

                if not self._running:
                    break

                # 处理待更新数据
                self._process_update()
                self._pending_update = False

    def _process_update(self):
        """处理数据更新（受保护方法）"""
        current_data = self._last_dot_data

        try:
            # 生成图像（耗时操作）
            image = self._render_dot(current_data)
            if image:
                self._original_image = image
                self._update_display()
        except Exception as e:
            print(f"Render error: {e}")

    def _render_dot(self, dot_data: str) -> Image.Image:
        """DOT数据渲染方法"""
        proc = subprocess.Popen(
            ['dot', '-Tpng'],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        stdout, stderr = proc.communicate(dot_data.encode())

        if proc.returncode != 0:
            raise RuntimeError(f"Graphviz error: {stderr.decode()}")

        return Image.open(BytesIO(stdout))


    def _update_display(self):
        """图像显示更新"""
        if self._original_image is None:
            return

        try:
            # 转换为Qt兼容格式
            qimg = self._pil_to_qimage(self._original_image)
            pixmap = QPixmap.fromImage(qimg)

            # 发射信号（跨线程安全）
            self.data_received.emit(pixmap)
        except Exception as e:
            print(f"Display error: {e}")

    @staticmethod
    def _pil_to_qimage(pil_img: Image.Image) -> QImage:
        """PIL图像转QImage（优化版）"""
        if pil_img.mode == 'RGBA':
            fmt = QImage.Format_RGBA8888
        else:
            pil_img = pil_img.convert('RGBA')
            fmt = QImage.Format_RGBA8888

        return QImage(
            pil_img.tobytes(),
            pil_img.width,
            pil_img.height,
            pil_img.width * 4,
            fmt
        )

    def receive_data(self, dot_data: str):
        """接收新数据（线程安全入口）"""
        with self._cond:
            if dot_data != self._last_dot_data:
                self._last_dot_data = dot_data
                self._pending_update = True
                self._cond.notify()

    def stop(self):
        """安全停止线程"""
        with self._cond:
            self._running = False
            self._cond.notify_all()  # 唤醒可能处于等待的线程
