# Crypto Module (crypto)

## What is it?

The crypto module provides implementations of two fundamental cryptographic algorithms: MD5 message digest and AES encryption/decryption.

## Why do you need it?

In data security scenarios, MD5 is used for verifying data integrity and generating unique identifiers, while AES is used for data encryption protection. The crypto module provides these two most commonly used cryptographic algorithms with simple and easy-to-use interfaces.

## Header Files

```cpp
#include <tbox/crypto/md5.h>   //! MD5 message digest
#include <tbox/crypto/aes.h>   //! AES encryption/decryption
```

## Core Classes and Interfaces

### MD5 — MD5 Message Digest

| Method | Description |
|------|------|
| `MD5()` | Constructor |
| `update(data, len)` | Feed plaintext data, can be called multiple times |
| `finish(digest)` | Finalize computation, output 16-byte digest |

> **Note**: After calling `finish()`, you cannot call `update()` again.

### AES — AES Encryption/Decryption

AES only implements single 16-byte block operations (AES-128).

| Method | Description |
|------|------|
| `AES(key)` | Constructor, takes a 16-byte key |
| `setKey(key)` | Set/change the key |
| `cipher(input, output)` | Encrypt a 16-byte block |
| `invcipher(input, output)` | Decrypt a 16-byte block |

> **Note**: Both input and output are 16 bytes in length, and the key is also 16 bytes.

## Usage Examples

> Full examples available in the header file inline examples and unit test cases

### MD5 Computation

```cpp
const char *str1 = "cpp-tbox, C++ Treasure Box,";
const char *str2 = " is an event-based service application development library.";

crypto::MD5 md5;
md5.update(str1, strlen(str1));  //! Can feed data in segments
md5.update(str2, strlen(str2));

uint8_t md5_digest[16];
md5.finish(md5_digest);  //! Get the 16-byte MD5 digest

//! Convert digest to readable hex string
char hex_str[33];
for (int i = 0; i < 16; ++i)
    snprintf(hex_str + i*2, 3, "%02x", md5_digest[i]);
LogInfo("MD5: %s", hex_str);
```

### AES Encryption and Decryption

```cpp
uint8_t key[16] = {0x01,0x02,...};       //! 16-byte key
uint8_t plain_text[16] = "Hello AES!..."; //! 16-byte plaintext
uint8_t cipher_text[16];                  //! Ciphertext output
uint8_t decrypted[16];                    //! Decrypted plaintext

crypto::AES aes(key);

//! Encrypt
aes.cipher(plain_text, cipher_text);

//! Decrypt
aes.invcipher(cipher_text, decrypted);

//! decrypted should match plain_text
```

## Common Scenarios

1. **Data Integrity Verification**: Compute a file's MD5 digest and compare it against a known digest
2. **Unique Identifier Generation**: Use MD5 to generate a unique ID from combined data
3. **Data Encryption Protection**: Use AES to encrypt sensitive data, transmit or store the ciphertext
4. **Key Rotation**: Use setKey() to switch keys without recreating the AES object

## Important Notes

1. **MD5 Security**: MD5 is no longer recommended for security authentication scenarios (collision attacks exist); use it only for checksums and identifier generation
2. **AES Single Block Only**: This implementation only handles 16-byte single blocks; for encrypting longer data, you need to implement CBC/CTR or other modes yourself
3. **Key Length**: Only supports 16-byte keys (AES-128); 24/32-byte keys are not supported
4. **finish() Finality**: After MD5's finish() is called, the object state is marked as finalized and update() cannot be called again
5. **Ciphertext Length**: AES encryption output is the same length as input (16 bytes), with no length increase

## Related Modules

- **base**: Provides basic type definitions
- **util**: Can be combined with Base64 to encode encryption results as text
