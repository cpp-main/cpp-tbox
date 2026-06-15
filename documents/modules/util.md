# Utility Module (util)

## What is it?

The util module provides 17+ general-purpose utility components, covering data processing, JSON parsing, serialization, encoding/decoding, process management, argument parsing, and more. These utilities are independent and lightweight, and can be used as needed.

## Why do you need it?

In C++ project development, you often need general-purpose utilities that are not in the standard library: binary data buffering, JSON configuration file parsing, command-line argument parsing, data serialization and deserialization, UUID generation, CRC checksums, etc. The util module encapsulates these commonly used utilities in a unified way, avoiding repetitive implementation in every project.

## Header Files

```cpp
// Data processing
#include <tbox/util/buffer.h>            //! Binary buffer

// JSON utilities
#include <tbox/util/json.h>             //! JSON parsing and field extraction
#include <tbox/util/json_deep_loader.h> //! JSON deep loading (supports __include__)

// Serialization
#include <tbox/util/serializer.h>       //! Serialization/deserialization (big/little endian)

// Encoding/decoding
#include <tbox/util/base64.h>           //! Base64 encoding/decoding
#include <tbox/util/crc.h>              //! CRC checksum
#include <tbox/util/checksum.h>         //! Checksum

// Variables and arguments
#include <tbox/util/variables.h>        //! Variable management object
#include <tbox/util/argument_parser.h>  //! Command-line argument parsing

// Process management
#include <tbox/util/pid_file.h>         //! PID file
#include <tbox/util/async_pipe.h>       //! Async pipe
#include <tbox/util/execute_cmd.h>      //! Execute command
#include <tbox/util/fs.h>               //! Filesystem utilities
#include <tbox/util/fd.h>               //! fd utilities
#include <tbox/util/split_cmdline.h>    //! Split command line

// Others
#include <tbox/util/uuid.h>             //! UUID generation
#include <tbox/util/timestamp.h>        //! Timestamp
#include <tbox/util/string.h>           //! String utilities
#include <tbox/util/string_to.h>        //! String conversion
#include <tbox/util/scalable_integer.h> //! Scalable integer
```

## Core Components

### Buffer -- Binary Buffer

Buffer is a read-write separated buffer, supporting append for writing and fetch for reading:

```
   buffer_ptr_                        buffer_size_
     |                                      |
     v                                      V
     +----+----------------+----------------+
     |    | readable bytes | writable bytes |
     +----+----------------+----------------+
          ^                ^
          |                |
      read_index_       write_index_
```

| Method | Description |
|--------|-------------|
| `append(data, size)` | Write data, returns actual written size |
| `fetch(buff, size)` | Read data, returns actual read size |
| `readableSize()` | Readable data size |
| `writableSize()` | Writable space size |
| `readableBegin()` | Readable area start address |
| `writableBegin()` | Writable area start address |
| `hasRead(size)` | Mark size bytes as read |
| `hasReadAll()` | Mark all data as read |
| `hasWritten(size)` | Mark size bytes as written |
| `ensureWritableSize(size)` | Ensure writable space |
| `reset()` | Reset the buffer |
| `shrink()` | Reduce excess capacity |

> **Note**: Buffer is not thread-safe; external locking is required for multi-threaded use.

### Json -- JSON Parsing and Field Extraction

Provides safe JSON field extraction functions:

```cpp
//! Extract field values from a Json object
bool Get(const Json &js, int &value);
bool Get(const Json &js, std::string &value);
bool GetField(const Json &js, "field_name", int &value);
bool GetField(const Json &js, "field_name", std::string &value);

//! Check field types
bool HasObjectField(const Json &js, "field");
bool HasArrayField(const Json &js, "field");
bool HasStringField(const Json &js, "field");
bool HasIntegerField(const Json &js, "field");

//! Parse JSON file
Json js = json::Load("config.json");        //! Exception-throwing version
bool ok = json::Load("config.json", js);    //! Non-throwing version
```

### DeepLoader -- JSON Deep Loading

Supports using `__include__` in JSON files to import other JSON files:

```json
// main.json
{
  "main.a": 1,
  "__include__": ["sub/sub1.json => sub1", "common.json"]
}
```

```cpp
Json js = json::LoadDeeply("main.json");  //! Automatically loads referenced files and merges them
```

> For a complete example, see `examples/util/json_deep_loader/`

### ArgumentParser -- Command-line Argument Parsing

Supports short arguments (-h) and long arguments (--help, --level=6):

```cpp
bool print_help = false;
int level = 0;

tbox::util::ArgumentParser parser(
    [&](char short_opt, const std::string &long_opt,
        ArgumentParser::OptionValue &opt_value) {
        if (short_opt == 'h' || long_opt == "help") {
            print_help = true;
        } else if (short_opt == 'l' || long_opt == "level") {
            level = std::stoi(opt_value.get());
        } else {
            cerr << "invalid option" << endl;
            return false;
        }
        return true;
    }
);

if (!parser.parse(argc, argv))
    return 0;
```

### Serializer / Deserializer -- Serialization

Supports big/little endian data serialization and deserialization, providing stream-style operations:

```cpp
std::vector<uint8_t> block;
Serializer s(block, Endian::kBig);

s << uint16_t(0x1234) << int32_t(42) << float(3.14);

Deserializer d(block.data(), block.size(), Endian::kBig);
uint16_t v1; int32_t v2; float v3;
d >> v1 >> v2 >> v3;
```

### Variables -- Variable Management

Variable management object, supporting hierarchical inheritance (parent lookup):

```cpp
Variables vars;
vars.define("name", Json("default"));
vars.set("name", Json("new_value"));

Json value;
vars.get("name", value);     //! Look up locally
vars.get("name", value, false); //! Continue lookup from parent

//! Set parent variable table
vars.setParent(&parent_vars);
```

### UUID -- UUID Generation

```cpp
std::string id = util::UUID::Generate();  //! Generate v4 UUID
```

### Base64 -- Base64 Encoding/Decoding

```cpp
std::string encoded = util::Base64::Encode(data, size);
std::vector<uint8_t> decoded = util::Base64::Decode(encoded);
```

### CRC -- CRC Checksum

```cpp
uint16_t crc16 = util::CRC::Calc16(data, size);
uint32_t crc32 = util::CRC::Calc32(data, size);
```

### PidFile -- PID File

Prevents duplicate startup of the same program:

```cpp
util::PidFile pid_file;
pid_file.setPathPrefix("/var/run/myapp");  //! Automatically creates PID file
pid_file.enable();
```

## Common Scenarios

1. **Configuration file loading**: Use json::Load or LoadDeeply to read JSON configuration
2. **Command-line arguments**: Use ArgumentParser to parse -h/--help/-l/--level etc.
3. **Binary protocol**: Use Buffer to buffer sent/received data, Serializer to serialize protocol fields
4. **Checksum and encoding**: CRC for data integrity, Base64 for encoding binary data
5. **Prevent duplicate startup**: Use PidFile to ensure only one process instance

## Important Notes

1. **Buffer is not thread-safe**: External locking is required for multi-threaded use
2. **Json::Load exceptions**: Load() throws OpenFileError or ParseJsonFileError; LoadDeeply also has DuplicateIncludeError
3. **Serializer byte order**: Pay attention to big/little endian alignment; default is big endian (commonly used for network protocols)
4. **ArgumentParser.get()**: After calling opt_value.get(), the parser will not mistake the value for an argument item
5. **DeepLoader duplicate prevention**: The same file cannot be included twice; it will throw DuplicateIncludeError

## Related Modules

- **base**: Provides Json (nlohmann/json) forward declarations and definitions
- **event**: AsyncPipe runs based on Loop
- **flow**: Action uses Variables to store variables
- **main**: Module uses Variables and json to load configuration
- **network**: Uses Buffer as the received data buffer
