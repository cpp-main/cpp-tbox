if ENABLE_INSTALLATION
lib_LTLIBRARIES = libtbox_flow.la
else
noinst_LTLIBRARIES = libtbox_flow.la
endif

MAINTAINERCLEANFILES = Makefile.in

VERSION_INFO = 1:0:1
GENERIC_LD_FLAGS = -version-info $(VERSION_INFO) $(AM_LDFLAGS)

includedir = @includedir@/tbox/flow

HEAD_FILES = \
			 state_machine.h \
			 action.h \
			 event.h \
			 action_executor.h \
			 event_subscriber.h \
			 event_publisher.h \
			 event_publisher_impl.h \
			 actions/function_action.h \
			 actions/sleep_action.h \
			 actions/sequence_action.h \
			 actions/parallel_action.h \
			 actions/invert_action.h \
			 actions/if_else_action.h \
			 actions/loop_action.h \
			 actions/loop_if_action.h \
			 actions/event_action.h

CPP_SRC_FILES = \
				state_machine.cpp \
				action.cpp \
				action_executor.cpp \
				event_publisher_impl.cpp \
				actions/function_action.cpp \
				actions/sleep_action.cpp \
				actions/sequence_action.cpp \
				actions/parallel_action.cpp \
				actions/invert_action.cpp \
				actions/if_else_action.cpp \
				actions/loop_action.cpp \
				actions/loop_if_action.cpp \
				actions/event_action.cpp

ADDITIONAL_CXXFLAGS = -DLOG_MODULE_ID='"tbox_flow"'

libtbox_flow_la_SOURCES = $(CPP_SRC_FILES)
libtbox_flow_la_CXXFLAGS = $(ADDITIONAL_CXXFLAGS)
libtbox_flow_la_LDFLAGS = $(LDFLAGS) $(GENERIC_LD_FLAGS)


if ENABLE_INSTALLATION
include_HEADERS = $(HEAD_FILES)
else
noinst_HEADERS = $(HEAD_FILES)
endif
