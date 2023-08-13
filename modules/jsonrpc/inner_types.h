#ifndef TBOX_JSONRPC_INNER_TYPES_H_20230813
#define TBOX_JSONRPC_INNER_TYPES_H_20230813

namespace tbox {
namespace jsonrpc {

enum ErrorCode {
    kParseError     = -32700,
    kInvalidRequest = -32600,
    kMethodNotFound = -32601,
    kInvalidParams  = -32602,
    kInternalError  = -32603,
    kRequestTimeout = -32000,
};

}
}

#endif //TBOX_JSONRPC_INNER_TYPES_H_20230813
