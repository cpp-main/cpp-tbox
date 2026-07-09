/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\  .
 *  \\     \     \ /\  /
 *   \\     \     \   /
 *    \\     \     \ /
 *     -============'
 *
 * Copyright (c) 2026 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "html_text.h"

const std::string kEchoBinHtml =
R"rawliteral(<!DOCTYPE html>
<html lang="zh">
<head>
<meta charset="utf-8">
<title>WebSocket 二进制 Echo</title>
<style>
* { box-sizing: border-box; }
body { font-family: sans-serif; margin: 0; padding: 0; background: #1a1a2e;
       min-height: 100vh; color: #e0e0e0; }

/* 主容器 */
#app { max-width: 640px; width: 100%; margin: 20px auto; }

/* 头部 */
#header { background: #16213e; border-radius: 12px; padding: 16px 20px;
          margin-bottom: 16px; display: flex; justify-content: space-between;
          align-items: center; }
#header h2 { margin: 0; color: #00d4ff; font-size: 20px; }
#status { font-size: 13px; }
#status.connected { color: #4caf50; }
#status.disconnected { color: #f44336; }

/* 发送按钮区 */
#send-bar { background: #16213e; border-radius: 12px; padding: 16px 20px;
            margin-bottom: 16px; }
#send-bar h3 { margin: 0 0 12px 0; color: #00d4ff; font-size: 15px; }
.btn-group { display: flex; gap: 10px; flex-wrap: wrap; }
.btn-group button { padding: 10px 18px; border: none; border-radius: 8px;
                    font-size: 14px; cursor: pointer; font-weight: bold;
                    transition: background .2s; }
.btn-16  { background: #0f3460; color: #00d4ff; }
.btn-16:hover { background: #1a5276; }
.btn-256 { background: #533483; color: #fff; }
.btn-256:hover { background: #6c3d94; }
.btn-1k  { background: #e94560; color: #fff; }
.btn-1k:hover { background: #f06878; }
.btn-group button:disabled { opacity: 0.4; cursor: not-allowed; }

/* 统计区 */
#stats { background: #16213e; border-radius: 12px; padding: 16px 20px;
         margin-bottom: 16px; }
#stats h3 { margin: 0 0 12px 0; color: #00d4ff; font-size: 15px; }
.stats-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
.stat-item { background: #0f3460; border-radius: 8px; padding: 10px 14px;
             text-align: center; }
.stat-item .label { font-size: 12px; color: #888; }
.stat-item .value { font-size: 18px; color: #00d4ff; font-weight: bold; }

/* 日志区 */
#log-area { background: #16213e; border-radius: 12px; padding: 16px 20px; }
#log-area h3 { margin: 0 0 12px 0; color: #00d4ff; font-size: 15px; }
#log { list-style: none; margin: 0; padding: 0; height: 300px; overflow-y: auto;
      border: 1px solid #0f3460; border-radius: 8px; background: #0a0a1a;
      padding: 8px; }
#log li { padding: 4px 8px; margin: 2px 0; border-radius: 4px; font-size: 13px;
          word-break: break-all; }
#log li.echo    { color: #4caf50; }
#log li.stat    { color: #00d4ff; }
#log li.sent    { color: #ff9800; }
#log li.warn    { color: #f44336; }
#log li.text    { color: #aaa; }

/* 页脚 */
#footer { position: fixed; bottom: 10px; right: 14px; font-size: 12px; color: #555; }
#footer a { color: #00d4ff; text-decoration: none; }
#footer a:hover { text-decoration: underline; }
</style>
</head>
<body>

<div id="app">
  <div id="header">
    <h2>⬡ WebSocket 二进制 Echo</h2>
    <div id="status" class="disconnected">未连接</div>
  </div>

  <div id="send-bar">
    <h3>发送二进制数据</h3>
    <div class="btn-group">
      <button class="btn-16"  onclick="sendBin(16)"    id="btn16">16 字节</button>
      <button class="btn-256" onclick="sendBin(256)"   id="btn256">256 字节</button>
      <button class="btn-1k"  onclick="sendBin(1024)"  id="btn1k">1 KB</button>
    </div>
  </div>

  <div id="stats">
    <h3>统计信息</h3>
    <div class="stats-grid">
      <div class="stat-item"><div class="label">发送帧数</div><div class="value" id="sent-frames">0</div></div>
      <div class="stat-item"><div class="label">发送字节</div><div class="value" id="sent-bytes">0</div></div>
      <div class="stat-item"><div class="label">接收帧数</div><div class="value" id="recv-frames">0</div></div>
      <div class="stat-item"><div class="label">接收字节</div><div class="value" id="recv-bytes">0</div></div>
    </div>
    <div style="margin-top:10px; background:#0f3460; border-radius:8px; padding:10px 14px;">
      <div class="label" style="font-size:12px; color:#888;">服务器推送统计</div>
      <div id="server-stat" style="font-size:14px; color:#00d4ff; margin-top:4px;">等待推送...</div>
    </div>
  </div>

  <div id="log-area">
    <h3>通信日志</h3>
    <ul id="log"></ul>
  </div>
</div>

<div id="footer">
  Powered by <a href="https://github.com/cpp-main/cpp-tbox" target="_blank">cpp-tbox</a>
  · <a href="https://github.com/cpp-main/cpp-tbox/tree/master/examples/websocket/echo_bin" target="_blank">查看源码</a>
</div>

<script>
var ws = null;
var sentFrames = 0, sentBytes = 0;
var recvFrames = 0, recvBytes = 0;
var statusEl = document.getElementById('status');
var logUl = document.getElementById('log');

//! 统计帧头部标识 "STAT" = [0x53, 0x54, 0x41, 0x54]
var STAT_HEADER = new Uint8Array([0x53, 0x54, 0x41, 0x54]);

function addLog(msg, cls) {
  var li = document.createElement('li');
  li.className = cls || '';
  li.textContent = msg;
  logUl.appendChild(li);
  logUl.scrollTop = logUl.scrollHeight;
}

function updateStats() {
  document.getElementById('sent-frames').textContent = sentFrames;
  document.getElementById('sent-bytes').textContent  = sentBytes;
  document.getElementById('recv-frames').textContent = recvFrames;
  document.getElementById('recv-bytes').textContent  = recvBytes;
}

//! 生成指定大小的随机二进制数据并发送
function sendBin(size) {
  if (!ws || ws.readyState !== WebSocket.OPEN) {
    addLog('未连接，无法发送', 'warn');
    return;
  }

  //! 生成随机 Uint8Array
  var data = new Uint8Array(size);
  for (var i = 0; i < size; i++)
    data[i] = Math.floor(Math.random() * 256);

  ws.send(data);

  sentFrames++;
  sentBytes += size;
  updateStats();

  //! 显示前 8 个字节（十六进制）
  var preview = [];
  var showLen = Math.min(8, size);
  for (var i = 0; i < showLen; i++)
    preview.push(data[i].toString(16).toUpperCase().padStart(2, '0'));
  var suffix = size > 8 ? '...' : '';
  addLog('→ 发送 ' + size + 'B: [' + preview.join(' ') + suffix + ']', 'sent');
}

//! 连接 WebSocket
function connect() {
  ws = new WebSocket('ws://' + location.host + '/ws/echo');
  ws.binaryType = 'arraybuffer';  //! 关键：设置接收类型为 ArrayBuffer

  ws.onopen = function() {
    statusEl.textContent = '已连接';
    statusEl.className = 'connected';
    addLog('✓ 连接成功', 'stat');
    document.querySelectorAll('.btn-group button').forEach(function(b) { b.disabled = false; });
  };

  //! Close 代码描述映射（RFC 6455 Section 7.4）
  function closeCodeDesc(code) {
    var desc = {
      1000: '正常关闭',
      1001: '终端离开',
      1002: '协议错误',
      1003: '不支持的数据类型',
      1005: '无状态码（保留）',
      1006: '异常关闭（连接意外断开）',
      1007: '无效帧负载数据',
      1008: '策略违规',
      1009: '消息过大',
      1010: '缺少必要扩展',
      1011: '内部服务器错误',
      1012: '服务重启',
      1013: '稍后重试',
      1015: 'TLS握手失败'
    };
    return desc[code] || ('未知代码: ' + code);
  }

  ws.onclose = function(e) {
    //! CloseEvent 包含 code 和 reason，这是诊断问题的关键信息
    //! code 1010 = 缺少必要扩展；1006 = 异常关闭；1007 = 帧数据无效
    addLog('✗ 连接断开，代码: ' + e.code + ' (' + closeCodeDesc(e.code) + ')，原因: "' + e.reason + '"，wasClean: ' + e.wasClean, 'warn');
    statusEl.textContent = '已断开';
    statusEl.className = 'disconnected';
    document.querySelectorAll('.btn-group button').forEach(function(b) { b.disabled = true; });
  };

  ws.onerror = function(e) {
    //! onerror 事件本身不携带太多信息，记录事件类型和 readyState
    addLog('✗ 连接出错，事件类型: ' + e.type + '，readyState: ' + ws.readyState, 'warn');
  };

  ws.onmessage = function(e) {
    if (e.data instanceof ArrayBuffer) {
      //! 二进制帧
      var bytes = new Uint8Array(e.data);
      recvFrames++;
      recvBytes += bytes.length;
      updateStats();

      //! 判断是否为统计帧（头部为 "STAT"）
      if (bytes.length >= 4 &&
          bytes[0] === 0x53 && bytes[1] === 0x54 &&
          bytes[2] === 0x41 && bytes[3] === 0x54) {
        //! 解码：跳过 4 字节头部，剩余为 JSON 字符串
        var jsonStr = new TextDecoder().decode(bytes.subarray(4));
        try {
          var obj = JSON.parse(jsonStr);
          document.getElementById('server-stat').innerHTML =
            '收 ' + obj.recv_frames + ' 帧 / ' + obj.recv_bytes + ' B · '
            + '发 ' + obj.sent_frames + ' 帧 / ' + obj.sent_bytes + ' B · '
            + '在线 ' + obj.clients + ' 人';
          addLog('📊 服务器统计: ' + jsonStr, 'stat');
        } catch(ex) {
          addLog('📊 统计帧 JSON 解析失败', 'warn');
        }
      } else {
        //! Echo 回传帧：显示前 8 字节
        var preview = [];
        var showLen = Math.min(8, bytes.length);
        for (var i = 0; i < showLen; i++)
          preview.push(bytes[i].toString(16).toUpperCase().padStart(2, '0'));
        var suffix = bytes.length > 8 ? '...' : '';
        addLog('← Echo ' + bytes.length + 'B: [' + preview.join(' ') + suffix + ']', 'echo');
      }
    } else {
      //! 文本帧（如提示消息）
      addLog('← 文本: ' + e.data, 'text');
    }
  };
}

//! 页面加载后自动连接
connect();
</script>
</body>
</html>)rawliteral";
