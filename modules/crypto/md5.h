#ifndef TBOX_CRYPTO_MD5_H_20221218
#define TBOX_CRYPTO_MD5_H_20221218

#include <string>

namespace tbox {
namespace crypto {

class MD5 {
  public:
    MD5();

  public:
    void update(const void* plain_text_ptr, size_t plain_text_len);
    void finish(uint8_t digest[16]);

  private:
    uint32_t  count_[2];    //!< 记录当前状态，其数据位数
    uint32_t  state_[4];    //!< 4个数，一共32位 记录用于保存对512bits信息加密的中间结果或者最终结果
    uint8_t   buffer_[64];  //!< 一共64字节，512位
};

}
}

#endif //TBOX_CRYPTO_MD5_H_20221218
