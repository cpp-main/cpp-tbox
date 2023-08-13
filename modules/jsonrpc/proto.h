#ifndef TBOX_JSONRPC_PROTO_H_20230812
#define TBOX_JSONRPC_PROTO_H_20230812

#include <functional>
#include <tbox/base/json_fwd.h>

namespace tbox {
namespace jsonrpc {

class Proto {
  public:
    using RecvRequestCallback = std::function<void(int id, const std::string &method, const Json &params)>;
    using RecvRespondCallback = std::function<void(int id, int errcode, const Json &result)>;
    using SendDataCallback = std::function<void(const void* data_ptr, size_t data_size)>;

    struct Callbacks {
        RecvRequestCallback recv_request_cb;
        RecvRespondCallback recv_respond_cb;
        SendDataCallback    send_data_cb;
    };

    void setCallbacks(const Callbacks &cbs) { cbs_ = cbs; }

  public:
    void sendRequest(int id, const std::string &method);
    void sendRequest(int id, const std::string &method, const Json &js_params);

    void sendResult(int id, const Json &js_result);
    void sendError(int id, int errcode, const std::string &message = "");

  public:
    /**
     * 当传输层收到数据后调用。该方法进行解包然后进行后续的处理
     */
    virtual ssize_t onRecvData(const void *data_ptr, size_t data_size) = 0;

  protected:
    virtual void sendJson(const Json &js) = 0;

    void onRecvJson(const Json &js);

    Callbacks cbs_;
};

}
}

#endif //TBOX_JSONRPC_PROTO_H_20230812
