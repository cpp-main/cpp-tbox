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
#include "ws_server.h"
#include "ws_server_impl.h"

#include <tbox/base/log.h>
#include <tbox/base/assert.h>
#include <tbox/base/defines.h>
#include <tbox/base/wrapped_recorder.h>

#include <tbox/network/tcp_connection.h>
#include <tbox/crypto/sha1.h>
#include <tbox/util/base64.h>
#include <tbox/util/string.h>

#undef  MODULE_ID
#define MODULE_ID "tbox.ws"

namespace tbox {
namespace websocket {
namespace server {

using namespace std::placeholders;

WsServer::Impl::Impl(WsServer *wp_parent, event::Loop *wp_loop)
  : wp_parent_(wp_parent)
  , wp_loop_(wp_loop)
{ }

WsServer::Impl::~Impl()
{
    TBOX_ASSERT(cb_level_ == 0);
    cleanup();
}

bool WsServer::Impl::initialize(http::server::Server *http_server, const std::string &url_path)
{
    if (state_ != WsServer::State::kNone)
        return false;

    //! 记录 URL 路径
    url_path_ = url_path;

    //! 记录 HTTP 服务器指针（不立即注册中间件，等 start() 时注册）
    wp_http_server_ = http_server;

    state_ = WsServer::State::kInited;
    return true;
}

bool WsServer::Impl::start()
{
    if (state_ != WsServer::State::kInited)
        return false;

    //! 注册自身到 HTTP 服务器（WsServer::Impl 即为 Middleware）
    mw_token_ = wp_http_server_->use(this);

    state_ = WsServer::State::kRunning;
    return true;
}

void WsServer::Impl::stop()
{
    if (state_ != WsServer::State::kRunning)
        return;

    //! 从 HTTP 服务器反注册中间件
    wp_http_server_->unuse(mw_token_);
    mw_token_.reset();

    //! 清除 WsConnection 内部回调，防止断开时回调到 Impl
    ws_conns_.foreach([](WsConnection *conn) {
        conn->setCloseCallback(nullptr);
        conn->setTextMessageCallback(nullptr);
        conn->setBinaryMessageCallback(nullptr);
        conn->setErrorCallback(nullptr);
    });

    //! 删除所有 WsConnection（析构时会断开并延后删除 TcpConnection）
    ws_conns_.foreach([](WsConnection *conn) {
        CHECK_DELETE_OBJ(conn);
    });
    ws_conns_.clear();

    state_ = WsServer::State::kInited;
}

void WsServer::Impl::cleanup()
{
    if (state_ == WsServer::State::kNone)
        return;

    if (state_ == WsServer::State::kRunning)
        stop();

    wp_http_server_ = nullptr;

    state_ = WsServer::State::kNone;
}

//! === permessage-deflate 扩展协商解析 ===

//! 客户端 Sec-WebSocket-Extensions 头部中 permessage-deflate 扩展的解析结果
//! RFC 7692 Section 4.1: 扩展参数定义
struct WsExtOfferParams {
    bool found = false;                              //!< 是否找到 permessage-deflate 扩展
    bool server_no_context_takeover = false;         //!< 服务器不保持压缩上下文
    bool client_no_context_takeover = false;         //!< 客户端不保持压缩上下文
    bool server_max_window_bits_present = false;     //!< 是否包含 server_max_window_bits
    int  server_max_window_bits = 15;                //!< 服务器滑动窗口位数（默认15）
    bool client_max_window_bits_present = false;     //!< 是否包含 client_max_window_bits
    int  client_max_window_bits = 0;                 //!< 客户端滑动窗口位数，0=不带值(支持8~15)
};

//! 解析 Sec-WebSocket-Extensions 头部中的 permessage-deflate 扩展参数
//! 格式示例: "permessage-deflate; client_max_window_bits; server_max_window_bits=15"
//! 多个扩展以逗号分隔: "permessage-deflate; client_max_window_bits, x-other-ext"
static WsExtOfferParams ParseWsExtOffer(const std::string &ext_header)
{
    WsExtOfferParams params;

    //! 找到 permessage-deflate 扩展的起始位置（需完整匹配，非子串）
    static const std::string kExtName = "permessage-deflate";
    size_t pos = 0;
    while (pos < ext_header.size()) {
        size_t found_pos = ext_header.find(kExtName, pos);
        if (found_pos == std::string::npos)
            break;

        //! 前面应为逗号、空格或字符串开头；后面应为分号、逗号、空格或结尾
        bool valid_prefix = (found_pos == 0) ||
            (ext_header[found_pos - 1] == ',') ||
            (ext_header[found_pos - 1] == ' ');
        size_t name_end = found_pos + kExtName.size();
        bool valid_suffix = (name_end >= ext_header.size()) ||
            (ext_header[name_end] == ';') ||
            (ext_header[name_end] == ',') ||
            (ext_header[name_end] == ' ');
        if (valid_prefix && valid_suffix) {
            pos = found_pos;
            break;
        }
        pos = name_end;
    }

    if (pos >= ext_header.size())
        return params;

    params.found = true;

    //! 确定参数区域：扩展名之后到下一个扩展（逗号）或字符串结尾
    size_t param_start = pos + kExtName.size();
    size_t comma_pos = ext_header.find(',', param_start);
    size_t param_end = (comma_pos != std::string::npos) ? comma_pos : ext_header.size();

    //! 在参数区域内逐个解析分号分隔的参数
    std::string section = ext_header.substr(param_start, param_end - param_start);
    size_t search_pos = 0;
    while (search_pos < section.size()) {
        size_t semi_pos = section.find(';', search_pos);
        if (semi_pos == std::string::npos)
            break;

        //! 提取参数文本（跳过分号和空格）
        size_t text_start = semi_pos + 1;
        while (text_start < section.size() && section[text_start] == ' ')
            text_start++;

        //! 找到参数结束位置（下一个分号或区域结尾）
        size_t text_end = section.find(';', text_start);
        if (text_end == std::string::npos)
            text_end = section.size();

        //! 去掉尾部空格
        while (text_end > text_start && section[text_end - 1] == ' ')
            text_end--;

        std::string param_text = section.substr(text_start, text_end - text_start);
        if (param_text.empty()) {
            search_pos = text_end;
            continue;
        }

        //! 解析参数名=值
        size_t eq_pos = param_text.find('=');
        std::string param_name = (eq_pos != std::string::npos)
            ? param_text.substr(0, eq_pos) : param_text;
        std::string param_value = (eq_pos != std::string::npos)
            ? param_text.substr(eq_pos + 1) : "";

        //! 去掉参数名尾部空格和参数值首尾空格
        while (!param_name.empty() && param_name.back() == ' ')
            param_name.pop_back();
        while (!param_value.empty() && param_value.front() == ' ')
            param_value.erase(param_value.begin());
        while (!param_value.empty() && param_value.back() == ' ')
            param_value.pop_back();

        //! 匹配已知参数（RFC 7692 Section 4.1）
        if (param_name == "server_no_context_takeover") {
            params.server_no_context_takeover = true;
        } else if (param_name == "client_no_context_takeover") {
            params.client_no_context_takeover = true;
        } else if (param_name == "server_max_window_bits") {
            params.server_max_window_bits_present = true;
            if (!param_value.empty())
                params.server_max_window_bits = std::stoi(param_value);
        } else if (param_name == "client_max_window_bits") {
            params.client_max_window_bits_present = true;
            if (!param_value.empty())
                params.client_max_window_bits = std::stoi(param_value);
            else
                params.client_max_window_bits = 0; //!< 不带值，表示客户端支持 8~15
        }

        search_pos = text_end;
    }

    return params;
}

//! === Middleware 接口实现 ===

void WsServer::Impl::handle(http::server::ContextSptr sp_ctx, const http::server::NextFunc &next)
{
    auto &req = sp_ctx->req();

    if (IsWsUpgradeRequest(req)) {
        //! URL 路径匹配规则：
        //! - url_path_ 以 '/' 结尾：前缀匹配，如 "/api/" 匹配 "/api/aa"、" /api/bb/cc"
        //! - url_path_ 不以 '/' 结尾：全量匹配，如 "/api" 仅匹配 "/api"
        //! - url_path_ 为空字符串：匹配所有 WebSocket 升级请求
        if (!url_path_.empty()) {
            bool matched = false;
            if (url_path_.back() == '/') {
                //! 前缀匹配
                matched = util::string::IsStartWith(req.url.path, url_path_);
            } else {
                //! 全量匹配
                matched = (req.url.path == url_path_);
            }
            if (!matched) {
                //! 不是本服务关心的 URL，传递给下一个中间件
                next();
                return;
            }
        }

        LogDbg("ws upgrade request: %s", req.url.path.c_str());

        auto &res = sp_ctx->res();

        //! 设置 101 响应
        res.status_code = http::StatusCode::k101_SwitchingProtocols;
        res.http_ver = http::HttpVer::k1_1;

        //! 从请求头中获取 Upgrade 和 Connection 信息
        auto upgrade_iter = req.headers.find("Upgrade");
        if (upgrade_iter != req.headers.end())
            res.headers["Upgrade"] = upgrade_iter->second;
        else
            res.headers["Upgrade"] = "websocket";

        auto connection_iter = req.headers.find("Connection");
        if (connection_iter != req.headers.end())
            res.headers["Connection"] = connection_iter->second;
        else
            res.headers["Connection"] = "Upgrade";

        //! 计算 Sec-WebSocket-Accept
        auto key_iter = req.headers.find("Sec-WebSocket-Key");
        if (key_iter != req.headers.end())
            res.headers["Sec-WebSocket-Accept"] = ComputeWsAcceptKey(key_iter->second);

        //! RFC 7692：压缩扩展协商
        //! 若 server 允许压缩且客户端请求了 permessage-deflate，同意压缩
        bool compression_agreed = false;
        if (compression_config_.enabled) {
            auto ext_iter = req.headers.find("Sec-WebSocket-Extensions");
            if (ext_iter != req.headers.end()) {
                //! 解析客户端的 permessage-deflate 扩展参数
                WsExtOfferParams offer_params = ParseWsExtOffer(ext_iter->second);
                std::string resp_value;
                if (offer_params.found) {
                    //! 构建响应参数：
                    //! 1) server_no_context_takeover: 服务器每条消息独立压缩，必须声明
                    //! 2) client_no_context_takeover: 要求客户端每条消息独立压缩
                    //! 3) client_max_window_bits: 若客户端 offered，RFC 7692 MUST 包含
                    //!    否则 Chrome 等浏览器会关闭连接（RFC 7692 Section 4.3）
                    //! 4) server_max_window_bits: 若客户端 offered，可选包含（MAY）
                    resp_value = "permessage-deflate; server_no_context_takeover; client_no_context_takeover";

                    //! RFC 7692 Section 4.2.2:
                    //! "If a server received an extension offer containing the client_max_window_bits
                    //!  parameter, the server MUST include the client_max_window_bits parameter
                    //!  in the corresponding extension response."
                    if (offer_params.client_max_window_bits_present) {
                        //! 不带值(client_max_window_bits=0)表示客户端支持 8~15
                        //! 带值时须 ≤ 客户端 offered 值
                        //! 响应值同时须 ≤ 服务器 max_window_bits
                        int respond_bits = (offer_params.client_max_window_bits == 0)
                            ? compression_config_.max_window_bits
                            : std::min(offer_params.client_max_window_bits, compression_config_.max_window_bits);
                        if (respond_bits < 8)  respond_bits = 8;
                        if (respond_bits > 15) respond_bits = 15;
                        resp_value += "; client_max_window_bits=" + std::to_string(respond_bits);
                    }

                    //! RFC 7692 Section 4.2.2:
                    //! server_max_window_bits 为 MAY，非 MUST
                    //! 此处显式声明，方便客户端明确知道服务器使用的窗口位数
                    if (offer_params.server_max_window_bits_present) {
                        //! 响应值须 ≤ 客户端 offered 值，同时须 ≤ 服务器 max_window_bits
                        int respond_bits = std::min(offer_params.server_max_window_bits, compression_config_.max_window_bits);
                        if (respond_bits < 8)  respond_bits = 8;
                        if (respond_bits > 15) respond_bits = 15;
                        resp_value += "; server_max_window_bits=" + std::to_string(respond_bits);
                    }

                    compression_agreed = true;
                    LogDbg("ws compression agreed: %s", resp_value.c_str());
                }
                if (!resp_value.empty())
                    res.headers["Sec-WebSocket-Extensions"] = resp_value;
            }
        }

        //! 注册升级回调：HTTP 服务器发送 101 响应后，将 TcpConnection 交给 WsServer
        //! 同时传递压缩协商结果
        WsCompressionConfig conn_compress_config;
        if (compression_agreed) {
            conn_compress_config.enabled = true;
            conn_compress_config.no_context_takeover = compression_config_.no_context_takeover;
            conn_compress_config.max_window_bits = compression_config_.max_window_bits;
        }
        res.upgrade_cb = std::bind(&WsServer::Impl::onWsUpgrade, this, _1, req.url.path, conn_compress_config);

        //! 升级请求已处理，不再调用 next()
        return;
    }

    //! 非 WebSocket 升级请求，传递给下一个中间件
    next();
}

//! === 升级与连接管理 ===

void WsServer::Impl::onWsUpgrade(network::TcpConnection *tcp_conn, const std::string &url_path,
                                 const WsCompressionConfig &compress_config)
{
    RECORD_SCOPE();
    LogDbg("ws upgrade: new connection from %s", tcp_conn->peerAddr().toString().c_str());

    //! 创建 WsConnection，并存入 Cabinet（直接 alloc 并存入指针）
    //! 传入升级时的 URL 路径、压缩配置和分片大小
    WsConnection *ws_conn = new WsConnection(wp_loop_, tcp_conn, url_path, compress_config, fragment_size_);
    ConnToken ws_token = ws_conns_.alloc(ws_conn);

    //! 设置 WsConnection 的回调（bind 捕获 ConnToken，不传递 WsConnection*）
    ws_conn->setCloseCallback(std::bind(&WsServer::Impl::onWsDisconnected, this, ws_token));
    ws_conn->setTextMessageCallback(std::bind(&WsServer::Impl::onWsTextMessage, this, ws_token, _1));
    ws_conn->setBinaryMessageCallback(std::bind(&WsServer::Impl::onWsBinaryMessage, this, ws_token, _1));
    ws_conn->setErrorCallback(std::bind(&WsServer::Impl::onWsError, this, ws_token));

    //! 通知用户（传递 ConnToken）
    if (connected_cb_) {
        ++cb_level_;
        connected_cb_(ws_token);
        --cb_level_;
    }
}

void WsServer::Impl::onWsDisconnected(const ConnToken &client)
{
    RECORD_SCOPE();
    LogDbg("ws disconnected");

    //! 先通知用户（此时 ConnToken 在 Cabinet 中仍有效）
    //! 用户可通过 ConnToken 调用 WsServer 方法获取连接信息
    if (disconnected_cb_) {
        ++cb_level_;
        disconnected_cb_(client);
        --cb_level_;
    }

    //! 从 Cabinet 中移除并获取指针
    WsConnection *ws_conn = ws_conns_.free(client);

    //! 延后删除 WsConnection（确保回调中还能访问对象）
    wp_loop_->runNext([ws_conn] { CHECK_DELETE_OBJ(ws_conn); },
        "WsServer::onWsDisconnected, delete ws_conn");
}

void WsServer::Impl::onWsTextMessage(const ConnToken &client, std::string &&data)
{
    if (text_message_cb_) {
        ++cb_level_;
        text_message_cb_(client, std::move(data));
        --cb_level_;
    }
}

void WsServer::Impl::onWsBinaryMessage(const ConnToken &client, std::vector<uint8_t> &&data)
{
    if (binary_message_cb_) {
        ++cb_level_;
        binary_message_cb_(client, std::move(data));
        --cb_level_;
    }
}

void WsServer::Impl::onWsError(const ConnToken &client)
{
    if (error_cb_) {
        ++cb_level_;
        error_cb_(client);
        --cb_level_;
    }

    //! 出错后关闭连接
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        ws_conn->close();
}

//! === 通过 ConnToken 操作连接 ===

bool WsServer::Impl::send(const ConnToken &client, const std::string &text)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->send(text);
    return false;
}

bool WsServer::Impl::send(const ConnToken &client, const char *str)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->send(str);
    return false;
}

bool WsServer::Impl::send(const ConnToken &client, const void *data, size_t len)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->send(data, len);
    return false;
}

bool WsServer::Impl::send(const ConnToken &client, const std::vector<uint8_t> &data)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->send(data);
    return false;
}

bool WsServer::Impl::close(const ConnToken &client, uint16_t code, const std::string &reason)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->close(code, reason);
    return false;
}

bool WsServer::Impl::ping(const ConnToken &client, const std::string &data)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->ping(data);
    return false;
}

bool WsServer::Impl::pong(const ConnToken &client, const std::string &data)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->pong(data);
    return false;
}

bool WsServer::Impl::isClientValid(const ConnToken &client) const
{
    return ws_conns_.at(client) != nullptr;
}

network::SockAddr WsServer::Impl::peerAddr(const ConnToken &client) const
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->peerAddr();
    return network::SockAddr();
}

std::string WsServer::Impl::getUrl(const ConnToken &client) const
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->getUrl();
    return "";
}

void WsServer::Impl::setContext(const ConnToken &client, void *context, ContextDeleter &&deleter)
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        ws_conn->setContext(context, std::move(deleter));
}

void* WsServer::Impl::getContext(const ConnToken &client) const
{
    auto ws_conn = ws_conns_.at(client);
    if (ws_conn != nullptr)
        return ws_conn->getContext();
    return nullptr;
}

//! === 静态辅助方法 ===

bool WsServer::Impl::IsWsUpgradeRequest(const http::Request &req)
{
    //! RFC 6455 Section 4.1:
    //! 1) 必须是 GET 方法
    //! 2) 必须包含 Upgrade: websocket 头部
    //! 3) 必须包含 Connection: Upgrade 头部
    //! 4) 必须包含 Sec-WebSocket-Key 头部
    //! 5) 必须包含 Sec-WebSocket-Version: 13 头部

    if (req.method != http::Method::kGet)
        return false;

    auto upgrade_iter = req.headers.find("Upgrade");
    if (upgrade_iter == req.headers.end()
        || upgrade_iter->second.find("websocket") == std::string::npos)
        return false;

    auto connection_iter = req.headers.find("Connection");
    if (connection_iter == req.headers.end()
        || connection_iter->second.find("Upgrade") == std::string::npos)
        return false;

    if (req.headers.find("Sec-WebSocket-Key") == req.headers.end())
        return false;

    //! 检查版本号
    auto version_iter = req.headers.find("Sec-WebSocket-Version");
    if (version_iter == req.headers.end()
        || version_iter->second != "13")
        return false;

    return true;
}

std::string WsServer::Impl::ComputeWsAcceptKey(const std::string &sec_ws_key)
{
    //! RFC 6455 Section 4.2.2:
    //! Sec-WebSocket-Accept = Base64(SHA1(Sec-WebSocket-Key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"))
    static const std::string ws_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

    std::string combined = sec_ws_key + ws_guid;
    uint8_t digest[20];
    crypto::SHA1::Calc(combined.data(), combined.size(), digest);

    return util::base64::Encode(digest, 20);
}

void WsServer::Impl::setCompressionEnable(bool enable)
{
    compression_config_.enabled = enable;
}

WsServer::WsServer(event::Loop *wp_loop)
  : impl_(new Impl(this, wp_loop))
{
    TBOX_ASSERT(wp_loop != nullptr);
}

WsServer::~WsServer()
{
    CHECK_DELETE_RESET_OBJ(impl_);
}

void WsServer::setCompressionEnable(bool enable)
{
    impl_->setCompressionEnable(enable);
}

void WsServer::setFragmentSize(size_t size)
{
    impl_->setFragmentSize(size);
}

bool WsServer::initialize(http::server::Server *http_server, const std::string &url_path)
{
    TBOX_ASSERT(http_server != nullptr);
    return impl_->initialize(http_server, url_path);
}

bool WsServer::start()
{
    return impl_->start();
}

void WsServer::stop()
{
    impl_->stop();
}

void WsServer::cleanup()
{
    impl_->cleanup();
}

WsServer::State WsServer::state() const
{
    return impl_->state();
}

void WsServer::setConnectedCallback(const ConnectedCallback &cb)
{
    impl_->setConnectedCallback(cb);
}

void WsServer::setDisconnectedCallback(const DisconnectedCallback &cb)
{
    impl_->setDisconnectedCallback(cb);
}

void WsServer::setTextMessageCallback(const TextMessageCallback &cb)
{
    impl_->setTextMessageCallback(cb);
}

void WsServer::setBinaryMessageCallback(const BinaryMessageCallback &cb)
{
    impl_->setBinaryMessageCallback(cb);
}

void WsServer::setErrorCallback(const ErrorCallback &cb)
{
    impl_->setErrorCallback(cb);
}

bool WsServer::send(const ConnToken &client, const std::string &text)
{
    return impl_->send(client, text);
}

bool WsServer::send(const ConnToken &client, const char *str)
{
    return impl_->send(client, str);
}

bool WsServer::send(const ConnToken &client, const void *data, size_t len)
{
    return impl_->send(client, data, len);
}

bool WsServer::send(const ConnToken &client, const std::vector<uint8_t> &data)
{
    return impl_->send(client, data);
}

bool WsServer::close(const ConnToken &client, uint16_t code, const std::string &reason)
{
    return impl_->close(client, code, reason);
}

bool WsServer::ping(const ConnToken &client, const std::string &data)
{
    return impl_->ping(client, data);
}

bool WsServer::pong(const ConnToken &client, const std::string &data)
{
    return impl_->pong(client, data);
}

bool WsServer::isClientValid(const ConnToken &client) const
{
    return impl_->isClientValid(client);
}

network::SockAddr WsServer::peerAddr(const ConnToken &client) const
{
    return impl_->peerAddr(client);
}

std::string WsServer::getUrl(const ConnToken &client) const
{
    return impl_->getUrl(client);
}

void WsServer::setContext(const ConnToken &client, void *context, ContextDeleter &&deleter)
{
    impl_->setContext(client, context, std::move(deleter));
}

void* WsServer::getContext(const ConnToken &client) const
{
    return impl_->getContext(client);
}

}
}
}
