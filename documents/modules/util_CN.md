# 工具集模块 (util)

## 是什么？

util 模块提供了 17+ 个通用工具组件，涵盖数据处理、JSON 解析、序列化、编码解码、进程管理、参数解析等功能。这些工具独立且轻量，可按需使用。

## 为什么需要它？

在 C++ 项目开发中，经常需要一些通用但不在标准库中的工具：二进制数据缓冲、JSON 配置文件解析、命令行参数解析、数据序列化与反序列化、UUID 生成、CRC 校验等。util 模块将这些常用工具统一封装，避免每个项目重复实现。

## 头文件

```cpp
// 数据处理
#include <tbox/util/buffer.h>            //! 二进制缓冲区

// JSON 工具
#include <tbox/util/json.h>             //! JSON 解析与字段提取
#include <tbox/util/json_deep_loader.h> //! JSON 深度加载（支持 __include__）

// 序列化
#include <tbox/util/serializer.h>       //! 序列化/反序列化（大端/小端）

// 编码解码
#include <tbox/util/base64.h>           //! Base64 编解码
#include <tbox/util/crc.h>              //! CRC 校验
#include <tbox/util/checksum.h>         //! 校验和

// 变量与参数
#include <tbox/util/variables.h>        //! 变量管理对象
#include <tbox/util/argument_parser.h>  //! 命令行参数解析

// 进程管理
#include <tbox/util/pid_file.h>         //! PID 文件
#include <tbox/util/async_pipe.h>       //! 异步管道
#include <tbox/util/execute_cmd.h>      //! 执行命令
#include <tbox/util/fs.h>               //! 文件系统工具
#include <tbox/util/fd.h>               //! fd 工具
#include <tbox/util/split_cmdline.h>    //! 分割命令行

// 其他
#include <tbox/util/uuid.h>             //! UUID 生成
#include <tbox/util/timestamp.h>        //! 时间戳
#include <tbox/util/string.h>           //! 字串工具
#include <tbox/util/string_to.h>        //! 字串转换
#include <tbox/util/scalable_integer.h> //! 可缩放整数
```

## 核心组件

### Buffer — 二进制缓冲区

Buffer 是一个读写分离的缓冲区，支持 append 写入和 fetch 读取：

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

| 方法 | 说明 |
|------|------|
| `append(data, size)` | 写入数据，返回实际写入大小 |
| `fetch(buff, size)` | 读取数据，返回实际读取大小 |
| `readableSize()` | 可读数据大小 |
| `writableSize()` | 可写空间大小 |
| `readableBegin()` | 可读区首地址 |
| `writableBegin()` | 可写区首地址 |
| `hasRead(size)` | 标记已读 size 字节 |
| `hasReadAll()` | 标记已读全部数据 |
| `hasWritten(size)` | 标记已写 size 字节 |
| `ensureWritableSize(size)` | 保障可写空间 |
| `reset()` | 重置缓冲区 |
| `shrink()` | 缩减多余容量 |

> **注意**：Buffer 不是线程安全的，多线程使用需在外部加锁。

### Json — JSON 解析与字段提取

提供安全的 JSON 字段提取函数：

```cpp
//! 从 Json 对象中提取字段值
bool Get(const Json &js, int &value);
bool Get(const Json &js, std::string &value);
bool GetField(const Json &js, "field_name", int &value);
bool GetField(const Json &js, "field_name", std::string &value);

//! 检查字段类型
bool HasObjectField(const Json &js, "field");
bool HasArrayField(const Json &js, "field");
bool HasStringField(const Json &js, "field");
bool HasIntegerField(const Json &js, "field");

//! 解析 JSON 文件
Json js = json::Load("config.json");        //! 抛异常版本
bool ok = json::Load("config.json", js);    //! 不抛异常版本
```

### DeepLoader — JSON 深度加载

支持在 JSON 文件中使用 `__include__` 导入其他 JSON 文件：

```json
// main.json
{
  "main.a": 1,
  "__include__": ["sub/sub1.json => sub1", "common.json"]
}
```

```cpp
Json js = json::LoadDeeply("main.json");  //! 自动加载引用的文件并合并
```

> 完整示例见 `examples/util/json_deep_loader/`

### ArgumentParser — 命令行参数解析

支持短参数（-h）和长参数（--help、--level=6）：

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

### Serializer / Deserializer — 序列化

支持大端/小端的数据序列化与反序列化，提供流式操作：

```cpp
std::vector<uint8_t> block;
Serializer s(block, Endian::kBig);

s << uint16_t(0x1234) << int32_t(42) << float(3.14);

Deserializer d(block.data(), block.size(), Endian::kBig);
uint16_t v1; int32_t v2; float v3;
d >> v1 >> v2 >> v3;
```

### Variables — 变量管理

变量管理对象，支持层级继承（parent 查找）：

```cpp
Variables vars;
vars.define("name", Json("default"));
vars.set("name", Json("new_value"));

Json value;
vars.get("name", value);     //! 从本地查找
vars.get("name", value, false); //! 从 parent 继续查找

//! 设置父变量表
vars.setParent(&parent_vars);
```

### UUID — UUID 生成

```cpp
std::string id = util::UUID::Generate();  //! 生成 v4 UUID
```

### Base64 — Base64 编解码

```cpp
std::string encoded = util::Base64::Encode(data, size);
std::vector<uint8_t> decoded = util::Base64::Decode(encoded);
```

### CRC — CRC 校验

```cpp
uint16_t crc16 = util::CRC::Calc16(data, size);
uint32_t crc32 = util::CRC::Calc32(data, size);
```

### PidFile — PID 文件

防止同一程序重复启动：

```cpp
util::PidFile pid_file;
pid_file.setPathPrefix("/var/run/myapp");  //! 自动创建 PID 文件
pid_file.enable();
```

## 常见场景

1. **配置文件加载**：使用 json::Load 或 LoadDeeply 读取 JSON 配置
2. **命令行参数**：使用 ArgumentParser 解析 -h/--help/-l/--level 等
3. **二进制协议**：使用 Buffer 缓冲收发数据，Serializer 序列化协议字段
4. **校验与编码**：CRC 校验数据完整性，Base64 编码二进制数据
5. **防止重复启动**：使用 PidFile 确保只有一个进程实例

## 注意事项

1. **Buffer 非线程安全**：多线程使用需在外部加锁
2. **Json::Load 异常**：Load() 抛 OpenFileError 或 ParseJsonFileError，LoadDeeply 还有 DuplicateIncludeError
3. **Serializer 字节序**：注意大小端对齐，默认大端（网络协议常用）
4. **ArgumentParser.get()**：调用 opt_value.get() 后，解析器不会将该值误认为参数项
5. **DeepLoader 防重复**：同一文件不能被 include 两次，会抛 DuplicateIncludeError

## 相关模块

- **base**：提供 Json（nlohmann/json）前置声明和定义
- **event**：AsyncPipe 基于 Loop 运行
- **flow**：Action 使用 Variables 存储变量
- **main**：Module 使用 Variables 和 json 加载配置
- **network**：使用 Buffer 作为接收数据缓冲区
