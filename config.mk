# 核心模块
MODULES += base
MODULES += util
MODULES += event
MODULES += eventx
MODULES += log
MODULES += network
MODULES += terminal
MODULES += main
MODULES += run

## 非核心模块，请根据需要选择
MODULES += coroutine
MODULES += http
MODULES += mqtt
MODULES += flow
MODULES += alarm
MODULES += crypto
MODULES += dbus
MODULES += jsonrpc

## 第三方库依赖
THIRDPARTY += nlohmann
