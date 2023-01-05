MAINTAINERCLEANFILES = Makefile.in
SAMPLE_TARGETS =

if BUILD_ALARM
SAMPLE_TARGETS += alarm/cron_alarm/cron_alarm \
				 alarm/oneshot_alarm/oneshot_alarm \
				 alarm/weekly_alarm/weekly_alarm
endif

if BUILD_BASE
SAMPLE_TARGETS += base/assert/assert
endif

if BUILD_EVENT
SAMPLE_TARGETS += event/01_io/io \
				  event/02_timer/timer \
				  event/03_signal/signal \
				  event/04_run_in_loop/run_in_loop \
				  event/05_run_next_seq/run_next_seq \
				  event/06_stdin_timer_signal/std_in_timer_signal \
				  event/07_delay_delete/delay_delete \
				  event/08_calc_game/calc_game
endif

if BUILD_EVENTX
SAMPLE_TARGETS += eventx/thread_pool/thread_pool
endif

if BUILD_HTTP
SAMPLE_TARGETS += http/server/async_respond/async_respond \
				  http/server/router/router \
				  http/server/simple/simple
endif

if BUILD_MAIN
SAMPLE_TARGETS += main/01_one_app/one_app \
				  main/02_more_than_one_apps/more_than_one_apps \
				  main/03_nc_client_and_echo_server/nc_client_echo_server \
				  main/04_initiate_runtime_error/initiate_runtime_error \
				  main/05_loop_block/loop_block \
				  main/06_run_in_backend/run_in_backend \
				  main/sample/sample
endif

if BUILD_NETWORK
SAMPLE_TARGETS += network/buffered_fd/io_echo \
				  network/stdio_stream/01_stdin_out/stdin_out \
				  network/stdio_stream/02_stdio/stdio \
				  network/tcp_acceptor/tcp_echo/tcp_echo \
				  network/tcp_acceptor/tcp_nc_server/tcp_nc_server \
				  network/tcp_client/tcp_echo/tcp_echo \
				  network/tcp_client/tcp_hex_client/tcp_hex_client \
				  network/tcp_client/tcp_nc_client/tcp_nc_client \
				  network/tcp_connector/tcp_nc_client/tcp_nc_client \
				  network/tcp_connector/tcp_echo/tcp_echo \
				  network/tcp_server/tcp_echo/tcp_echo \
				  network/tcp_server/tcp_nc_server/tcp_nc_server \
				  network/uart/uart_to_uart/uart_to_uart \
				  network/uart/uart_tool/uart_tool \
				  network/udp_socket/ping_pong/ping_pong \
				  network/udp_socket/recv_only/recv_only \
				  network/udp_socket/request/request \
				  network/udp_socket/respond/respond \
				  network/udp_socket/send_only/send_only
endif

if BUILD_TERMINAL
SAMPLE_TARGETS += terminal/tcp_rpc/tcp_rpc \
				  terminal/telnetd/telnetd
endif

if BUILD_UTIL
SAMPLE_TARGETS += util/backtrace/backtrace
endif

if BUILD_MQTT
SAMPLE_TARGETS += mqtt/conn/conn \
				  mqtt/pub/pub \
				  mqtt/sub/sub
endif


LIB_TBOX_BASE = $(srcdir)/../base/.libs/libtbox_base.la
LIB_TBOX_ALARM = $(srcdir)/../alarm/.libs/libtbox_alarm.la
LIB_TBOX_EVENT = $(srcdir)/../event/.libs/libtbox_event.la
LIB_TBOX_EVENTX = $(srcdir)/../eventx/.libs/libtbox_eventx.la
LIB_TBOX_HTTP = $(srcdir)/../http/.libs/libtbox_http.la
LIB_TBOX_NETWORK = $(srcdir)/../network/.libs/libtbox_network.la
LIB_TBOX_LOG = $(srcdir)/../log/.libs/libtbox_log.la
LIB_TBOX_UTIL = $(srcdir)/../util/.libs/libtbox_util.la
LIB_TBOX_MAIN = $(srcdir)/../main/.libs/libtbox_main.la
LIB_TBOX_MQTT = $(srcdir)/../mqtt/.libs/libtbox_mqtt.la
LIB_TBOX_TERMINAL = $(srcdir)/../terminal/.libs/libtbox_terminal.la


noinst_PROGRAMS = $(SAMPLE_TARGETS)
AM_CPPFLAGS = -DLOG_MODULE_ID='"sample"'
TBOX_SAMPLE_GC_LD = -ldl -lpthread

######################## alarm examples ###############################
alarm_cron_alarm_cron_alarm_SOURCES = alarm/cron_alarm/main.cpp
alarm_cron_alarm_cron_alarm_LDADD = $(LIB_TBOX_ALARM) $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

alarm_oneshot_alarm_oneshot_alarm_SOURCES = alarm/oneshot_alarm/main.cpp
alarm_oneshot_alarm_oneshot_alarm_LDADD = $(LIB_TBOX_ALARM) $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

alarm_weekly_alarm_weekly_alarm_SOURCES = alarm/weekly_alarm/main.cpp
alarm_weekly_alarm_weekly_alarm_LDADD = $(LIB_TBOX_ALARM) $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

######################## base examples ###############################
base_assert_assert_SOURCES = base/assert/main.cpp
base_assert_assert_LDADD = $(LIB_TBOX_BASE)

######################## event examples ###############################
event_01_io_io_SOURCES = event/01_io/main.cpp
event_01_io_io_LDADD = $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

event_02_timer_timer_SOURCES = event/02_timer/main.cpp
event_02_timer_timer_LDADD = $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

event_03_signal_signal_SOURCES = event/03_signal/main.cpp
event_03_signal_signal_LDADD = $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

event_04_run_in_loop_run_in_loop_SOURCES = event/04_run_in_loop/main.cpp
event_04_run_in_loop_run_in_loop_LDADD = $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

event_05_run_next_seq_run_next_seq_SOURCES = event/05_run_next_seq/main.cpp
event_05_run_next_seq_run_next_seq_LDADD = $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

event_06_stdin_timer_signal_std_in_timer_signal_SOURCES = event/06_stdin_timer_signal/main.cpp
event_06_stdin_timer_signal_std_in_timer_signal_LDADD = $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

event_07_delay_delete_delay_delete_SOURCES = event/07_delay_delete/main.cpp
event_07_delay_delete_delay_delete_LDADD = $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)


event_08_calc_game_calc_game_SOURCES = event/08_calc_game/main.cpp \
									   event/08_calc_game/game_lite.cpp \
									   event/08_calc_game/game.cpp
event_08_calc_game_calc_game_LDADD = $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

######################## eventx examples ###############################
eventx_thread_pool_thread_pool_SOURCES = eventx/thread_pool/main.cpp
eventx_thread_pool_thread_pool_LDADD = $(LIB_TBOX_EVENTX) $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)


######################## http examples ###############################
http_server_async_respond_async_respond_SOURCES = http/server/async_respond/async_respond.cpp
http_server_async_respond_async_respond_LDADD = $(LIB_TBOX_HTTP) $(LIB_TBOX_NETWORK) $(LIB_TBOX_EVENTX) $(LIB_TBOX_EVENT) $(LIB_TBOX_LOG) $(LIB_TBOX_UTIL) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

http_server_router_router_SOURCES = http/server/router/router.cpp
http_server_router_router_LDADD = $(LIB_TBOX_HTTP) $(LIB_TBOX_NETWORK) $(LIB_TBOX_EVENTX) $(LIB_TBOX_EVENT) $(LIB_TBOX_LOG) $(LIB_TBOX_UTIL) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

http_server_simple_simple_SOURCES = http/server/simple/simple.cpp
http_server_simple_simple_LDADD = $(LIB_TBOX_HTTP) $(LIB_TBOX_NETWORK) $(LIB_TBOX_EVENTX) $(LIB_TBOX_EVENT) $(LIB_TBOX_LOG) $(LIB_TBOX_UTIL) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)


######################## main examples ###############################

MAIN_SAMPLE_LDADD = $(LIB_TBOX_MAIN) \
					$(LIB_TBOX_TERMINAL) \
					$(LIB_TBOX_NETWORK) \
					$(LIB_TBOX_EVENTX) \
					$(LIB_TBOX_EVENT) \
					$(LIB_TBOX_LOG) \
					$(LIB_TBOX_UTIL) \
					$(LIB_TBOX_BASE) \
					$(TBOX_SAMPLE_GC_LD)

main_01_one_app_one_app_SOURCES = main/01_one_app/app.cpp \
								  main/01_one_app/main.cpp
main_01_one_app_one_app_LDADD = $(MAIN_SAMPLE_LDADD)

main_02_more_than_one_apps_more_than_one_apps_SOURCES = main/02_more_than_one_apps/main.cpp \
												main/02_more_than_one_apps/app1/app.cpp \
												main/02_more_than_one_apps/app1/sub.cpp \
												main/02_more_than_one_apps/app2/app.cpp \
												main/02_more_than_one_apps/app3/app.cpp
main_02_more_than_one_apps_more_than_one_apps_LDADD = $(MAIN_SAMPLE_LDADD)

main_03_nc_client_and_echo_server_nc_client_echo_server_SOURCES = main/03_nc_client_and_echo_server/echo_server/app.cpp \
																  main/03_nc_client_and_echo_server/main.cpp \
																  main/03_nc_client_and_echo_server/nc_client/app.cpp
main_03_nc_client_and_echo_server_nc_client_echo_server_LDADD = $(MAIN_SAMPLE_LDADD)

main_04_initiate_runtime_error_initiate_runtime_error_SOURCES = main/04_initiate_runtime_error/main.cpp
main_04_initiate_runtime_error_initiate_runtime_error_LDADD = $(MAIN_SAMPLE_LDADD)

main_05_loop_block_loop_block_SOURCES = main/05_loop_block/app.cpp \
										main/05_loop_block/main.cpp
main_05_loop_block_loop_block_LDADD = $(MAIN_SAMPLE_LDADD)

main_06_run_in_backend_run_in_backend_SOURCES = main/06_run_in_backend/app.cpp \
												main/06_run_in_backend/app_main.cpp \
												main/06_run_in_backend/main.cpp
main_06_run_in_backend_run_in_backend_LDADD = $(MAIN_SAMPLE_LDADD)

main_sample_sample_SOURCES = main/sample/app_main.cpp \
							 main/sample/build_time.cpp \
							 main/sample/app1/app.cpp \
							 main/sample/app2/app.cpp
main_sample_sample_LDADD = $(MAIN_SAMPLE_LDADD)



######################## Network examples ###############################
NETWORK_SAMPLE_LDADD = $(LIB_TBOX_NETWORK) $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(LIB_TBOX_UTIL) $(TBOX_SAMPLE_GC_LD)
network_buffered_fd_io_echo_SOURCES = network/buffered_fd/io_echo.cpp
network_buffered_fd_io_echo_LDADD = $(NETWORK_SAMPLE_LDADD)

network_stdio_stream_01_stdin_out_stdin_out_SOURCES = network/stdio_stream/01_stdin_out/main.cpp
network_stdio_stream_01_stdin_out_stdin_out_LDADD = $(NETWORK_SAMPLE_LDADD)

network_stdio_stream_02_stdio_stdio_SOURCES = network/stdio_stream/02_stdio/main.cpp
network_stdio_stream_02_stdio_stdio_LDADD = $(NETWORK_SAMPLE_LDADD)

network_tcp_acceptor_tcp_echo_tcp_echo_SOURCES = network/tcp_acceptor/tcp_echo/tcp_echo.cpp
network_tcp_acceptor_tcp_echo_tcp_echo_LDADD = $(NETWORK_SAMPLE_LDADD)

network_tcp_acceptor_tcp_nc_server_tcp_nc_server_SOURCES = network/tcp_acceptor/tcp_nc_server/tcp_nc_server.cpp
network_tcp_acceptor_tcp_nc_server_tcp_nc_server_LDADD = $(NETWORK_SAMPLE_LDADD)

network_tcp_client_tcp_echo_tcp_echo_SOURCES = network/tcp_client/tcp_echo/tcp_echo.cpp
network_tcp_client_tcp_echo_tcp_echo_LDADD = $(NETWORK_SAMPLE_LDADD)

network_tcp_client_tcp_hex_client_tcp_hex_client_SOURCES = network/tcp_client/tcp_hex_client/tcp_hex_client.cpp
network_tcp_client_tcp_hex_client_tcp_hex_client_LDADD = $(NETWORK_SAMPLE_LDADD)

network_tcp_client_tcp_nc_client_tcp_nc_client_SOURCES = network/tcp_client/tcp_nc_client/tcp_nc_client.cpp
network_tcp_client_tcp_nc_client_tcp_nc_client_LDADD = $(NETWORK_SAMPLE_LDADD)

network_tcp_connector_tcp_nc_client_tcp_nc_client_SOURCES = network/tcp_connector/tcp_nc_client/tcp_nc_client.cpp
network_tcp_connector_tcp_nc_client_tcp_nc_client_LDADD = $(NETWORK_SAMPLE_LDADD)

network_tcp_connector_tcp_echo_tcp_echo_SOURCES = network/tcp_connector/tcp_echo/tcp_echo.cpp
network_tcp_connector_tcp_echo_tcp_echo_LDADD = $(NETWORK_SAMPLE_LDADD)

network_tcp_server_tcp_echo_tcp_echo_SOURCES = network/tcp_server/tcp_echo/tcp_echo.cpp
network_tcp_server_tcp_echo_tcp_echo_LDADD = $(NETWORK_SAMPLE_LDADD)

network_tcp_server_tcp_nc_server_tcp_nc_server_SOURCES = network/tcp_server/tcp_nc_server/tcp_nc_server.cpp
network_tcp_server_tcp_nc_server_tcp_nc_server_LDADD = $(NETWORK_SAMPLE_LDADD)

network_uart_uart_to_uart_uart_to_uart_SOURCES = network/uart/uart_to_uart/uart_to_uart.cpp
network_uart_uart_to_uart_uart_to_uart_LDADD = $(NETWORK_SAMPLE_LDADD)

network_uart_uart_tool_uart_tool_SOURCES = network/uart/uart_tool/uart_tool.cpp
network_uart_uart_tool_uart_tool_LDADD = $(NETWORK_SAMPLE_LDADD)

network_udp_socket_ping_pong_ping_pong_SOURCES = network/udp_socket/ping_pong/ping_pong.cpp
network_udp_socket_ping_pong_ping_pong_LDADD = $(NETWORK_SAMPLE_LDADD)

network_udp_socket_recv_only_recv_only_SOURCES = network/udp_socket/recv_only/recv_only.cpp
network_udp_socket_recv_only_recv_only_LDADD = $(NETWORK_SAMPLE_LDADD)

network_udp_socket_request_request_SOURCES = network/udp_socket/request/request.cpp
network_udp_socket_request_request_LDADD = $(NETWORK_SAMPLE_LDADD)

network_udp_socket_respond_respond_SOURCES = network/udp_socket/respond/respond.cpp
network_udp_socket_respond_respond_LDADD = $(NETWORK_SAMPLE_LDADD)

network_udp_socket_send_only_send_only_SOURCES = network/udp_socket/send_only/send_only.cpp
network_udp_socket_send_only_send_only_LDADD = $(NETWORK_SAMPLE_LDADD)


######################## terminal examples ###############################
TERMINAL_SAMPLE_LDADD = $(LIB_TBOX_TERMINAL) $(LIB_TBOX_NETWORK) $(LIB_TBOX_EVENT) $(LIB_TBOX_UTIL) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)
terminal_tcp_rpc_tcp_rpc_SOURCES = terminal/tcp_rpc/main.cpp
terminal_tcp_rpc_tcp_rpc_LDADD = $(TERMINAL_SAMPLE_LDADD)

terminal_telnetd_telnetd_SOURCES = terminal/telnetd/main.cpp
terminal_telnetd_telnetd_LDADD = $(TERMINAL_SAMPLE_LDADD)

########################### util examples ###############################
util_backtrace_backtrace_SOURCES = util/backtrace/sample.cpp
util_backtrace_backtrace_LDADD = $(LIB_TBOX_UTIL) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD)

########################### mqttt examples ###############################
MQTT_SAMPLE_LDADD = $(LIB_TBOX_MQTT) $(LIB_TBOX_EVENT) $(LIB_TBOX_BASE) $(TBOX_SAMPLE_GC_LD) -lmosquitto
mqtt_conn_conn_SOURCES = mqtt/conn/conn.cpp
mqtt_conn_conn_LDADD = $(MQTT_SAMPLE_LDADD)

mqtt_pub_pub_SOURCES = mqtt/pub/pub.cpp
mqtt_pub_pub_LDADD = $(MQTT_SAMPLE_LDADD)

mqtt_sub_sub_SOURCES = mqtt/sub/sub.cpp
mqtt_sub_sub_LDADD = $(MQTT_SAMPLE_LDADD)

