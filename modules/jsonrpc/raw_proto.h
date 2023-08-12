#ifndef TBOX_JSONRPC_RAW_PROTO_H_20230812
#define TBOX_JSONRPC_RAW_PROTO_H_20230812

#include "proto.h"

namespace tbox {
namespace jsonrpc {

class RawProto : public Proto {
  public:
    virtual ssize_t onRecvData(const void *data_ptr, size_t data_size) override;

  protected:
    virtual void sendJson(const Json &js) override;
};

}
}

#endif //TBOX_JSONRPC_RAW_PROTO_H_20230812
