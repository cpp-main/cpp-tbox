export TOP_DIR := $(PWD)

export STAGING_DIR ?= $(TOP_DIR)/.staging
export STAGING_INCLUDE := $(STAGING_DIR)/include
export STAGING_LIB := $(STAGING_DIR)/lib

export INSTALL_DIR := $(TOP_DIR)/.install
export INSTALL_LIB := $(INSTALL_DIR)/lib

export BUILD_DIR := $(TOP_DIR)/.build

CCFLAGS := -I$(STAGING_INCLUDE)

export CFLAGS := $(CCFLAGS) -std=c99
export CXXFLAGS := $(CCFLAGS) -std=c++11
export LDFLAGS := -L$(STAGING_LIB) -L$(INSTALL_LIB)

TOOLCHAIN_BIN_PREFIX :=
export AR := $(TOOLCHAIN_BIN_PREFIX)ar
export AS := $(TOOLCHAIN_BIN_PREFIX)as
export CXX := $(TOOLCHAIN_BIN_PREFIX)g++
export CC := $(TOOLCHAIN_BIN_PREFIX)gcc
export NM := $(TOOLCHAIN_BIN_PREFIX)nm
export OBJCOPY := $(TOOLCHAIN_BIN_PREFIX)objcopy
export OBJDUMP := $(TOOLCHAIN_BIN_PREFIX)objdump
export STRINGS := $(TOOLCHAIN_BIN_PREFIX)strings
export SSTRIP := $(TOOLCHAIN_BIN_PREFIX)sstrip
export LSTRIP := $(TOOLCHAIN_BIN_PREFIX)lstrip
export STRIP := $(TOOLCHAIN_BIN_PREFIX)strip
