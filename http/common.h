#ifndef TBOX_HTTP_COMMON_H_20220501
#define TBOX_HTTP_COMMON_H_20220501

namespace tbox {
namespace http {

enum class Ver {
    kHttp_1_0,
    kHttp_1_1,
    kHttp_1_2,
};

//! 方法
enum class Method {
    kGet,
    kHead,
    kPut,
    kPost, 
    kDelete
};

//! 返回状态码
enum class StatusCode {
    k200_OK,
    k401_Unauthorized,
    k404_NotFound
};

}
}

#endif //TBOX_HTTP_COMMON_H_20220501
