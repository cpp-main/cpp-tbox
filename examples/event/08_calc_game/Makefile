EXE_NAME := example/event/08_calc_game

CPP_SRC_FILES := \
	main.cpp \
	game.cpp \

CXXFLAGS := -DLOG_MODULE_ID='"$(EXE_NAME)"' $(CXXFLAGS)
LDFLAGS += \
	-ltbox_event \
	-ltbox_base \

include ${TOP_DIR}/tools/exe_common.mk
