#ifndef TBOX_JSONRPC_RPC_H
#define TBOX_JSONRPC_RPC_H

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <tbox/base/json_fwd.h>
#include <tbox/event/forward.h>
#include <tbox/eventx/timeout_monitor.hpp>

namespace tbox {
namespace jsonrpc {

class Proto;

class Rpc {
  public:
    using RequestCallback = std::function<void(int errcode, const Json &js_result)>;
    using ServiceCallback = std::function<bool(int id, const Json &js_params, int &errcode, Json &js_result)>;

  public:
    explicit Rpc(event::Loop *loop);
    virtual ~Rpc();

    bool initialize(Proto *proto);
    void cleanup();

    void request(const std::string &method, const Json &js_params, RequestCallback &&cb);  //! 发送请求或消息，如果cb==nullptr，则是消息

    void registeService(const std::string &method, ServiceCallback &&cb);  //! 注册当方法被调用时回调什么
    void respond(int id, int errcode, const Json &js_result);

  protected:
    void onRecvRequest(int id, const std::string &method, const Json &params);
    void onRecvRespond(int id, int errcode, const Json &result);
    void onRequestTimeout(int id);
    void onRespondTimeout(int id);

  private:
    event::Loop *loop_;
    Proto *proto_ = nullptr;

    std::unordered_map<std::string, ServiceCallback> method_services_;

    int id_alloc_ = 0;
    std::unordered_map<int, RequestCallback> request_callback_;
    std::unordered_set<int> tobe_respond_;
    eventx::TimeoutMonitor<int> request_timeout_;   //! 请求超时监测
    eventx::TimeoutMonitor<int> respond_timeout_;   //! 回复超时监测
};


}
}

#endif //TBOX_JSONRPC_RPC_H
