MAINTAINERCLEANFILES = Makefile.in
TEST_TARGETS =
if BUILD_BASE
TEST_TARGETS +=  test_base
endif

if BUILD_ALARM
TEST_TARGETS += test_alarm
endif

if BUILD_EVENT
TEST_TARGETS += test_event
endif

if BUILD_EVENTX
TEST_TARGETS += test_eventx
endif

if BUILD_COROUTINE
TEST_TARGETS += test_coroutine
endif

if BUILD_CRYPTO
TEST_TARGETS += test_crypto
endif

if BUILD_FLOW
TEST_TARGETS += test_flow
endif

if BUILD_HTTP
TEST_TARGETS += test_http
endif

if BUILD_LOG
TEST_TARGETS += test_log
endif

if BUILD_TERMINAL
TEST_TARGETS += test_terminal
endif

if BUILD_UTIL
TEST_TARGETS += test_util
endif

GENERIC_LDLIBS = -lgtest -lgtest_main -lpthread

noinst_PROGRAMS = $(TEST_TARGETS)

#####################################
# test_base
#####################################
test_base_SOURCES = base/log_output_test.cpp \
					base/scope_exit_test.cpp \
					base/cabinet_test.cpp \
					base/json_test.cpp \
					base/lifetime_tag_test.cpp

test_base_CPPFLAGS = -DLOG_MODULE_ID='"base"'
test_base_LDADD = $(srcdir)/../base/.libs/libtbox_base.la
test_base_LDADD += $(GENERIC_LDLIBS)

#####################################
# test_alarm
#####################################
test_alarm_SOURCES = alarm/workday_calendar_test.cpp
test_alarm_CPPFLAGS = -I $(srcdir)/../alarm
test_alarm_LDADD = $(srcdir)/../event/.libs/libtbox_event.la
test_alarm_LDADD += $(srcdir)/../alarm/.libs/libtbox_alarm.la
test_alarm_LDADD += $(srcdir)/../base/.libs/libtbox_base.la
test_alarm_LDADD += $(GENERIC_LDLIBS)


####################
# test_event
####################
test_event_SOURCES = event/common_loop_test.cpp \
					 event/fd_event_test.cpp \
					 event/timer_event_test.cpp \
					 event/signal_event_test.cpp \
					 event/stat_test.cpp

test_event_CPPFLAGS = -DLOG_MODULE_ID='"event"'
test_event_CPPFLAGS += -I $(srcdir)/../event
test_event_LDADD = $(srcdir)/../event/.libs/libtbox_event.la
test_event_LDADD += $(srcdir)/../base/.libs/libtbox_base.la
test_event_LDADD += $(GENERIC_LDLIBS)

####################
# test_eventx
####################

test_eventx_SOURCES = eventx/thread_pool_test.cpp \
					  eventx/timer_pool_test.cpp \
					  eventx/timeout_monitor_test.cpp \
					  eventx/request_pool_test.cpp \
					  eventx/loop_wdog_test.cpp

test_eventx_CPPFLAGS = -DLOG_MODULE_ID='"eventx"'
test_eventx_CPPFLAGS += -I $(srcdir)/../eventx
test_eventx_LDADD = $(srcdir)/../event/.libs/libtbox_event.la
test_eventx_LDADD += $(srcdir)/../eventx/.libs/libtbox_eventx.la
test_eventx_LDADD += $(srcdir)/../base/.libs/libtbox_base.la
test_eventx_LDADD += $(GENERIC_LDLIBS)

####################
# test_coroutine
####################

test_coroutine_SOURCES = coroutine/scheduler_test.cpp \
						 coroutine/channel_test.cpp \
						 coroutine/semaphore_test.cpp \
						 coroutine/mutex_test.cpp \
						 coroutine/broadcast_test.cpp \
						 coroutine/condition_test.cpp

test_coroutine_CPPFLAGS = -DLOG_MODULE_ID='"coroutine"'
test_coroutine_CPPFLAGS += -I $(srcdir)/../coroutine
test_coroutine_LDADD = $(srcdir)/../event/.libs/libtbox_event.la
test_coroutine_LDADD += $(srcdir)/../coroutine/.libs/libtbox_coroutine.la
test_coroutine_LDADD += $(srcdir)/../base/.libs/libtbox_base.la
test_coroutine_LDADD += $(GENERIC_LDLIBS)

#####################################
# test_crypto
#####################################
test_crypto_SOURCES = crypto/md5_test.cpp \
					  crypto/aes_test.cpp

test_crypto_CPPFLAGS = -DLOG_MODULE_ID='"crypto"'
test_crypto_CPPFLAGS += -I $(srcdir)/../crypto
test_crypto_LDADD = $(srcdir)/../crypto/.libs/libtbox_crypto.la
test_crypto_LDADD += $(srcdir)/../util/.libs/libtbox_util.la
test_crypto_LDADD += $(srcdir)/../base/.libs/libtbox_base.la
test_crypto_LDADD += $(GENERIC_LDLIBS) -ldl

#####################################
# test_flow
#####################################
test_flow_SOURCES = flow/state_machine_test.cpp \
	flow/action_test.cpp \
	flow/event_publisher_impl_test.cpp \
	flow/action_executor_test.cpp \
	flow/actions/sleep_action_test.cpp \
	flow/actions/function_action_test.cpp \
	flow/actions/sequence_action_test.cpp \
	flow/actions/parallel_action_test.cpp \
	flow/actions/invert_action_test.cpp \
	flow/actions/if_else_action_test.cpp \
	flow/actions/loop_action_test.cpp \
	flow/actions/loop_if_action_test.cpp

test_flow_CPPFLAGS = -DLOG_MODULE_ID='"crypto"'
test_flow_CPPFLAGS += -I $(srcdir)/../flow
test_flow_CPPFLAGS += -I $(srcdir)/../flow/actions
test_flow_LDADD = $(srcdir)/../flow/.libs/libtbox_flow.la
test_flow_LDADD += $(srcdir)/../event/.libs/libtbox_event.la
test_flow_LDADD += $(srcdir)/../base/.libs/libtbox_base.la
test_flow_LDADD += $(GENERIC_LDLIBS)

#####################################
# test_http
#####################################
test_http_SOURCES = http/common_test.cpp \
					http/respond_test.cpp \
					http/request_test.cpp \
					http/url_test.cpp \
					http/server/request_parser_test.cpp


test_http_CPPFLAGS = -DLOG_MODULE_ID='"http"'
test_http_CPPFLAGS += -I $(srcdir)/../http
test_http_CPPFLAGS += -I $(srcdir)/../http/server
test_http_LDADD = $(srcdir)/../http/.libs/libtbox_http.la
test_http_LDADD += $(srcdir)/../network/.libs/libtbox_network.la
test_http_LDADD += $(srcdir)/../log/.libs/libtbox_log.la
test_http_LDADD += $(srcdir)/../event/.libs/libtbox_event.la
test_http_LDADD += $(srcdir)/../util/.libs/libtbox_util.la
test_http_LDADD += $(srcdir)/../base/.libs/libtbox_base.la
test_http_LDADD += $(GENERIC_LDLIBS) -ldl

#####################################
# test_log
#####################################
test_log_SOURCES = \
				   log/stdout_channel_test.cpp \
				   log/syslog_channel_test.cpp \
				   log/async_channel_test.cpp \
				   log/file_async_channel_test.cpp

test_log_CPPFLAGS = -DLOG_MODULE_ID='"log"'
test_log_CPPFLAGS += -I $(srcdir)/../log
test_log_LDADD = $(srcdir)/../log/.libs/libtbox_log.la
test_log_LDADD += $(srcdir)/../util/.libs/libtbox_util.la
test_log_LDADD += $(srcdir)/../event/.libs/libtbox_event.la
test_log_LDADD += $(srcdir)/../base/.libs/libtbox_base.la
test_log_LDADD += $(GENERIC_LDLIBS) -ldl

#####################################
# test_terminal
#####################################
test_terminal_SOURCES = terminal/impl/key_event_scanner_test.cpp

test_terminal_CPPFLAGS = -DLOG_MODULE_ID='"terminal"'
test_terminal_CPPFLAGS += -I $(srcdir)/../terminal/impl
test_terminal_LDADD = $(srcdir)/../terminal/.libs/libtbox_terminal.la
test_terminal_LDADD += $(srcdir)/../util/.libs/libtbox_util.la
test_terminal_LDADD += $(srcdir)/../network/.libs/libtbox_network.la
test_terminal_LDADD += $(srcdir)/../event/.libs/libtbox_event.la
test_terminal_LDADD += $(srcdir)/../base/.libs/libtbox_base.la
test_terminal_LDADD += $(GENERIC_LDLIBS) -ldl

#####################################
# test_util
#####################################
test_util_SOURCES = util/pid_file_test.cpp \
					util/string_test.cpp \
					util/fs_test.cpp \
					util/argument_parser_test.cpp \
					util/split_cmdline_test.cpp \
					util/serializer_test.cpp \
					util/time_counter_test.cpp \
					util/async_pipe_test.cpp \
					util/json_test.cpp \
					util/base64_test.cpp \
					util/checksum_test.cpp \
					util/crc_test.cpp

test_util_CPPFLAGS = -DLOG_MODULE_ID='"util"'
test_util_CPPFLAGS += -I $(srcdir)/../util
test_util_LDADD = $(srcdir)/../util/.libs/libtbox_util.la
test_util_LDADD += $(srcdir)/../base/.libs/libtbox_base.la
test_util_LDADD += $(GENERIC_LDLIBS) -ldl

