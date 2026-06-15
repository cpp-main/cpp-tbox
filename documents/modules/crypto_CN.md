# 加密模块 (crypto)

## 是什么？

crypto 模块提供了 MD5 消息摘要和 AES 加密/解密两种基础加密算法的实现。

## 为什么需要它？

在数据安全场景中，MD5 用于校验数据完整性和生成唯一标识，AES 用于数据加密保护。crypto 模块提供了这两种最常用的加密算法，接口简洁易用。

## 头文件

```cpp
#include <tbox/crypto/md5.h>   //! MD5 消息摘要
#include <tbox/crypto/aes.h>   //! AES 加密/解密
```

## 核心类与接口

### MD5 — MD5 消息摘要

| 方法 | 说明 |
|------|------|
| `MD5()` | 构造 |
| `update(data, len)` | 喂入明文数据，可多次调用 |
| `finish(digest)` | 结束运算，输出16字节摘要 |

> **注意**：`finish()` 后不可再调用 `update()`。

### AES — AES 加密/解密

AES 仅实现单个 16 字节块的运算（AES-128）。

| 方法 | 说明 |
|------|------|
| `AES(key)` | 构造，传入16字节密钥 |
| `setKey(key)` | 设置/更换密钥 |
| `cipher(input, output)` | 加密16字节块 |
| `invcipher(input, output)` | 解密16字节块 |

> **注意**：input/output 均为 16 字节长度，密钥也是 16 字节。

## 使用示例

> 完整示例见头文件内联示例和单元测试用例

### MD5 计算

```cpp
const char *str1 = "cpp-tbox, C++ Treasure Box,";
const char *str2 = " is an event-based service application development library.";

crypto::MD5 md5;
md5.update(str1, strlen(str1));  //! 可分段喂入
md5.update(str2, strlen(str2));

uint8_t md5_digest[16];
md5.finish(md5_digest);  //! 得到16字节MD5摘要

//! 将摘要转为可读字串
char hex_str[33];
for (int i = 0; i < 16; ++i)
    snprintf(hex_str + i*2, 3, "%02x", md5_digest[i]);
LogInfo("MD5: %s", hex_str);
```

### AES 加密与解密

```cpp
uint8_t key[16] = {0x01,0x02,...};       //! 16字节密钥
uint8_t plain_text[16] = "Hello AES!..."; //! 16字节明文
uint8_t cipher_text[16];                  //! 密文输出
uint8_t decrypted[16];                    //! 解密后明文

crypto::AES aes(key);

//! 加密
aes.cipher(plain_text, cipher_text);

//! 解密
aes.invcipher(cipher_text, decrypted);

//! decrypted 应与 plain_text 一致
```

## 常见场景

1. **数据完整性校验**：计算文件的 MD5 摘要，与已知摘要比对
2. **唯一标识生成**：用 MD5 对组合数据生成唯一 ID
3. **数据加密保护**：使用 AES 加密敏感数据，传输或存储密文
4. **密钥更换**：使用 setKey() 切换密钥，无需重新创建 AES 对象

## 注意事项

1. **MD5 安全性**：MD5 已不推荐用于安全认证场景（存在碰撞攻击），建议仅用于校验和标识生成
2. **AES 仅单块**：本实现仅处理16字节单块，如需加密长数据需自行实现 CBC/CTR 等模式
3. **密钥长度**：仅支持 16 字节密钥（AES-128），不支持 24/32 字节密钥
4. **finish() 后不可再用**：MD5 的 finish() 调用后对象状态被标记为已完成，不可再 update()
5. **密文长度**：AES 加密输出与输入等长（16字节），不增加长度

## 相关模块

- **base**：提供基础类型定义
- **util**：可配合 Base64 将加密结果编码为文本
