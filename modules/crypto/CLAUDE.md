# CLAUDE.md - crypto 模块

## 模块定位

`crypto` 提供常用的密码学算法实现（纯 C++ 自实现，无第三方加密库依赖）。

## 依赖关系

- 上游依赖：`util`、`base`
- 被依赖：websocket（SHA1 用于 WebSocket 握手）

## 关键组件

| 文件 | 说明 |
|------|------|
| `md5.h` | `MD5`：`update()` 分段喂入明文，`finish(digest[16])` 输出 16 字节摘要 |
| `sha1.h` | `SHA1`：`update()`/`finish(digest[20])`，另有静态 `SHA1::Calc()` 一次性接口 |
| `aes.h` | `AES`：`cipher()` 加密 / `invcipher()` 解密，仅实现单个 16 字节块运算 |

## 注意事项

- `MD5`/`SHA1` 均支持分段 `update()` 后一次性 `finish()`。
- `SHA1` 注释明确：已不推荐用于安全目的，仅用于 WebSocket 握手等非安全场景。
- `AES` 仅单块（16 字节）运算，不支持分块/填充，需调用方自行处理长数据与补齐。

## 测试

- 测试文件：`md5_test.cpp`、`aes_test.cpp`、`sha1_test.cpp`
- 运行：`.build/crypto/test`
