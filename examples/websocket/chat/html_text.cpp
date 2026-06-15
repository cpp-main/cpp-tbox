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
 * Copyright (c) 2025 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#include "html_text.h"

const std::string kChatHtml =
R"rawliteral(<!DOCTYPE html>
<html lang="zh">
<head>
<meta charset="utf-8">
<title>WebSocket 群聊</title>
<style>
* { box-sizing: border-box; }
body { font-family: sans-serif; margin: 0; padding: 0; background: #f5f5f5;
       min-height: 100vh; }

/* 登录页 */
#login { display: flex; justify-content: center; align-items: center; min-height: 100vh; }
#login-card { background: #fff; border-radius: 12px; padding: 40px 30px;
              box-shadow: 0 4px 20px rgba(0,0,0,0.12); text-align: center; width: 340px; }
#login-card h2 { margin: 0 0 24px 0; color: #1976d2; font-size: 22px; }
#login-card input[type=text] { width: 100%; padding: 12px 16px; border: 2px solid #e0e0e0;
                   border-radius: 8px; font-size: 15px; margin-bottom: 20px;
                   transition: border-color .2s; }
#login-card input[type=text]:focus { outline: none; border-color: #1976d2; }

/* 聊天室选择 */
.room-select { margin-bottom: 20px; }
.room-select label { display: block; padding: 10px 0; font-size: 14px; color: #333;
                     cursor: pointer; }
.room-select input[type=radio] { margin-right: 8px; accent-color: #1976d2; }
.room-select input[type=radio]:not(:checked) + span { color: #999; }

#login-card button { width: 100%; padding: 12px; background: #1976d2; color: #fff;
                     border: none; border-radius: 8px; font-size: 16px; cursor: pointer;
                     font-weight: bold; transition: background .2s; }
#login-card button:hover { background: #1565c0; }
#login-card button:disabled { background: #90caf9; cursor: not-allowed; }
#login-status { margin-top: 14px; font-size: 13px; color: #666; min-height: 20px; }

/* 聊天页 */
#chat { display: none; max-width: 600px; width: 100%; margin: 0 auto; background: #fff;
        border-radius: 12px; box-shadow: 0 2px 12px rgba(0,0,0,0.08); overflow: hidden;
        margin-top: 20px; }
#chat-header { background: #1976d2; color: #fff; padding: 12px 16px;
               display: flex; justify-content: space-between; align-items: center; }
#chat-header .name { font-weight: bold; font-size: 16px; }
#chat-header .room-tag { font-size: 12px; background: rgba(255,255,255,0.2);
                         padding: 2px 8px; border-radius: 4px; margin-left: 8px; }
#chat-header .actions { display: flex; align-items: center; gap: 12px; }
#leave-btn { background: rgba(255,255,255,0.2); color: #fff; border: 1px solid rgba(255,255,255,0.4);
             padding: 4px 12px; border-radius: 6px; font-size: 13px; cursor: pointer;
             transition: background .2s; }
#leave-btn:hover { background: rgba(255,255,255,0.35); }
#messages { list-style: none; margin: 0; padding: 12px; height: 400px; overflow-y: auto;
           border-bottom: 1px solid #eee; }
#messages li { padding: 8px 12px; margin: 3px 0; border-radius: 6px; word-break: break-all;
              font-size: 14px; }
#messages li.notice { background: #e8f5e9; color: #2e7d32; text-align: center; font-size: 13px; }
#messages li.msg { background: #e3f2fd; color: #1565c0; }
#input-bar { display: flex; padding: 12px; }
#input-bar input { flex: 1; padding: 10px 14px; border: 1px solid #ddd; border-radius: 6px;
                   font-size: 14px; }
#input-bar input:focus { outline: none; border-color: #1976d2; }
#input-bar button { margin-left: 10px; padding: 10px 20px; background: #1976d2; color: #fff;
                    border: none; border-radius: 6px; cursor: pointer; font-size: 14px; }
#input-bar button:hover { background: #1565c0; }

/* 页脚 */
#footer { position: fixed; bottom: 10px; right: 14px; font-size: 12px; color: #bbb; }
#footer a { color: #1976d2; text-decoration: none; }
#footer a:hover { text-decoration: underline; }
</style>
</head>
<body>

<!-- 登录页 -->
<div id="login">
  <div id="login-card">
    <h2>🟢 WebSocket 群聊</h2>
    <input id="username" type="text" placeholder="请输入用户名" maxlength="20" autofocus>
    <div class="room-select">
      <label><input type="radio" name="room" value="chat-1" checked><span>聊天室 1</span></label>
      <label><input type="radio" name="room" value="chat-2"><span>聊天室 2</span></label>
    </div>
    <button id="login-btn">登录</button>
    <div id="login-status"></div>
  </div>
</div>

<!-- 聊天页（初始隐藏） -->
<div id="chat">
  <div id="chat-header">
    <div>
      <span class="name" id="chat-name"></span>
      <span class="room-tag" id="chat-room-tag"></span>
    </div>
    <div class="actions">
      <button id="leave-btn">离开</button>
    </div>
  </div>
  <ul id="messages"></ul>
  <div id="input-bar">
    <input id="msg" type="text" placeholder="输入消息...">
    <button id="send">发送</button>
  </div>
</div>

<!-- 页脚 -->
<div id="footer">
  Powered by <a href="https://github.com/cpp-main/cpp-tbox" target="_blank">cpp-tbox</a>
  · <a href="https://github.com/cpp-main/cpp-tbox/tree/master/examples/websocket/chat" target="_blank">查看源码</a>
</div>

<script>
var ws = null;
var username = '';
var room = '';
var loginView = document.getElementById('login');
var chatView = document.getElementById('chat');
var ul = document.getElementById('messages');
var msgInput = document.getElementById('msg');
var loginStatus = document.getElementById('login-status');
var chatName = document.getElementById('chat-name');
var chatRoomTag = document.getElementById('chat-room-tag');
var loginBtn = document.getElementById('login-btn');

function showLogin(msg, color) {
  chatView.style.display = 'none';
  loginView.style.display = 'flex';
  loginBtn.disabled = false;
  if (msg) {
    loginStatus.textContent = msg;
    loginStatus.style.color = color || '#c62828';
  } else {
    loginStatus.textContent = '';
  }
}

function showChat() {
  loginView.style.display = 'none';
  chatView.style.display = 'block';
  chatName.textContent = username;
  chatRoomTag.textContent = room === 'chat-1' ? '聊天室1' : '聊天室2';
  msgInput.focus();
}

//! 点击登录：选择聊天室，创建 WebSocket 连接并发送用户名
function doLogin() {
  username = document.getElementById('username').value.trim();
  if (!username) {
    loginStatus.textContent = '请输入用户名';
    loginStatus.style.color = '#c62828';
    return;
  }

  //! 获取选中的聊天室
  var radios = document.getElementsByName('room');
  for (var i = 0; i < radios.length; i++) {
    if (radios[i].checked) {
      room = radios[i].value;
      break;
    }
  }

  loginBtn.disabled = true;
  loginStatus.textContent = '连接中...';
  loginStatus.style.color = '#666';

  ws = new WebSocket('ws://' + location.host + '/ws/' + room);

  ws.onopen = function() {
    showChat();
    //! 第一条消息发送用户名，作为登录标识
    ws.send(username);
  };

  ws.onclose = function() {
    ul.innerHTML = '';
    showLogin('连接断开，请重新登录', '#c62828');
  };

  ws.onerror = function() {
    ul.innerHTML = '';
    showLogin('连接出错，请重试', '#c62828');
  };

  ws.onmessage = function(e) {
    var li = document.createElement('li');
    var text = e.data;
    if (text.indexOf('上线') !== -1 || text.indexOf('下线') !== -1) {
      li.className = 'notice';
    } else {
      li.className = 'msg';
    }
    li.textContent = text;
    ul.appendChild(li);
    ul.scrollTop = ul.scrollHeight;
  };
}

//! 主动离开聊天室
function doLeave() {
  if (ws) {
    ws.close();
    ws = null;
  }
  ul.innerHTML = '';
  showLogin();
}

function sendMsg() {
  var text = msgInput.value.trim();
  if (text && ws && ws.readyState === WebSocket.OPEN) {
    ws.send(text);
    msgInput.value = '';
  }
}

loginBtn.onclick = doLogin;
document.getElementById('username').onkeydown = function(e) {
  if (e.key === 'Enter') doLogin();
};
document.getElementById('send').onclick = sendMsg;
msgInput.onkeydown = function(e) { if (e.key === 'Enter') sendMsg(); };
document.getElementById('leave-btn').onclick = doLeave;
</script>
</body>
</html>)rawliteral";
