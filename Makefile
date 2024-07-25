#
#     .============.
#    //  M A K E  / \
#   //  C++ DEV  /   \
#  //  E A S Y  /  \/ \
# ++ ----------.  \/\  .
#  \\     \     \ /\  /
#   \\     \     \   /
#    \\     \     \ /
#     -============'
#
# Copyright (c) 2018 Hevake and contributors, all rights reserved.
#
# This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
# Use of this source code is governed by MIT license that can be found
# in the LICENSE file in the root of the source tree. All contributing
# project authors may be found in the CONTRIBUTORS.md file in the root
# of the source tree.
#

export TOP_DIR:=$(PWD)

include build_env.mk
include version.mk

.PHONY: all 3rd-party modules tools examples test clean distclean print

CCFLAGS := -Wall

RELEASE ?= 0
ENABLE_ASAN ?= 0
ENABLE_GPROF ?= 0

CCFLAGS += \
	-DTBOX_VERSION_MAJOR=$(TBOX_VERSION_MAJOR) \
	-DTBOX_VERSION_MINOR=$(TBOX_VERSION_MINOR) \
	-DTBOX_VERSION_REVISION=$(TBOX_VERSION_REVISION)

ifeq ($(RELEASE), 1)
CCFLAGS += -O2 -Os
else
CCFLAGS += -DDEBUG=1 -O0 -ggdb
ifeq ($(ENABLE_ASAN), 1)
CCFLAGS += -fsanitize=address -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address -static-libasan
endif
ifeq ($(ENABLE_GPROF), 1)
CCFLAGS += -pg
LDFLAGS += -pg
endif
endif

export CC CXX CFLAGS CXXFLAGS LDFLAGS APPS_DIR
export MODULES THIRDPARTY

include config.mk

CXXFLAGS := $(CCFLAGS) $(CXXFLAGS)
CFLAGS := $(CCFLAGS) $(CFLAGS)
APPS_DIR := $(PWD)

all: 3rd-party modules test tools examples

print:
	@echo "CXXFLAGS = $(CXXFLAGS)"
	@echo "LDFLAGS  = $(LDFLAGS)"

modules 3rd-party:
	$(MAKE) -C $@

test: modules
	$(MAKE) -C modules test

tools examples: modules
	$(MAKE) -C $@

run_test : test
	@for m in ${MODULES}; do $(BUILD_DIR)/$$m/test; done

clean:
	-rm -rf $(BUILD_DIR)

distclean: clean
	-rm -rf $(STAGING_DIR) $(INSTALL_DIR)
