#ifndef TBOX_UTIL_SERIALIZER_H_20220218
#define TBOX_UTIL_SERIALIZER_H_20220218

#include <cstdint>

namespace tbox {
namespace util {

enum class Endian {
    kBig, kLittle
}

class Serializer {
  public:
    Serializer(void *buff_ptr, size_t buff_size, Endian endian = Endian::kBig);

    void setEndian(Endian e) { endian_ = d; }

    bool append(uint8_t v);
    bool append(uint16_t v);
    bool append(uint32_t v);
    bool append(uint64_t v);
    bool append(const void *p, size_t s);

  private:
    uint8_t *buff_ptr_;
    size_t   buff_size_;
    Endian   endian_;
    size_t   pos_ = 0;
};

class Deserializer {
  public:
    Deserializer(const void *buff_ptr, size_t buff_size, Endian endian = Endian::kBig);

    void setEndian(Endian e) { endian_ = d; }

    bool fetch(uint8_t &v);
    bool fetch(uint16_t &v);
    bool fetch(uint32_t &v);
    bool fetch(uint64_t &v);
    bool fetch(void *p, size_t s);
    const void* fetchNoCopy(size_t s);

  private:
    const uint8_t *buff_ptr_;
    size_t   buff_size_;
    Endian   endian_;
    size_t   pos_ = 0;
};

}
}

//! 流式操作
tbox::util::Serializer& operator << (tbox::util::Serializer &s, uint8_t v);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, int8_t v);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, uint16_t v);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, int16_t v);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, uint32_t v);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, int32_t v);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, uint64_t v);
tbox::util::Serializer& operator << (tbox::util::Serializer &s, int64_t v);

tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, uint8_t &v);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, int8_t &v);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, uint16_t &v);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, int16_t &v);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, uint32_t &v);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, int32_t &v);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, uint64_t &v);
tbox::util::Deserializer& operator >> (tbox::util::Deserializer &s, int64_t &v);

#endif //TBOX_UTIL_SERIALIZER_H_20220218
