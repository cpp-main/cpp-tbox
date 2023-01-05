AC_INIT([cpp-tbox], [1.0.0])
AC_PREREQ([2.67])
AC_CONFIG_SRCDIR([tbox])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_PREFIX_DEFAULT([/usr/local])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST


AM_INIT_AUTOMAKE([subdir-objects])
AM_SILENT_RULES([yes])


#=====================================================
# program checks
#=====================================================

LT_INIT
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_LN_S

#=====================================================
# Optional features
#=====================================================
AC_ARG_ENABLE(base,
              AC_HELP_STRING([--enable-base], [build base module, @<:@default=yes@:>@]),
              [build_base=${enableval}],
              [build_base="yes"])

AC_ARG_ENABLE(util,
              AC_HELP_STRING([--enable-util], [build util module, @<:@default=yes@:>@]),
              [build_util=${enableval}],
              [build_util="yes"])

AC_ARG_ENABLE(event,
              AC_HELP_STRING([--enable-event], [build event module, @<:@default=yes@:>@]),
              [build_event=${enableval}],
              [build_event="yes"])

AC_ARG_ENABLE(eventx,
              AC_HELP_STRING([--enable-eventx], [build eventx module, @<:@default=yes@:>@]),
              [build_eventx=${enableval}],
              [build_eventx="yes"])

AC_ARG_ENABLE(log,
              AC_HELP_STRING([--enable-log], [build log module, @<:@default=yes@:>@]),
              [build_log=${enableval}],
              [build_log="yes"])

AC_ARG_ENABLE(network,
              AC_HELP_STRING([--enable-network], [build network module, @<:@default=yes@:>@]),
              [build_network=${enableval}],
              [build_network="yes"])

AC_ARG_ENABLE(http,
              AC_HELP_STRING([--enable-http], [build http module, @<:@default=yes@:>@]),
              [build_http=${enableval}],
              [build_http="yes"])

AC_ARG_ENABLE(coroutine,
              AC_HELP_STRING([--enable-coroutine], [build coroutine module, @<:@default=yes@:>@]),
              [build_coroutine=${enableval}],
              [build_coroutine="yes"])

AC_ARG_ENABLE(terminal,
              AC_HELP_STRING([--enable-terminal], [build terminal module, @<:@default=yes@:>@]),
              [build_terminal=${enableval}],
              [build_terminal="yes"])

AC_ARG_ENABLE(mqtt,
              AC_HELP_STRING([--enable-mqtt], [build mqtt module, @<:@default=no@:>@]),
              [build_mqtt=${enableval}],
              [build_mqtt="no"])

AC_ARG_ENABLE(main,
              AC_HELP_STRING([--enable-main], [build mian module, @<:@default=yes@:>@]),
              [build_main=${enableval}],
              [build_main="yes"])

AC_ARG_ENABLE(flow,
              AC_HELP_STRING([--enable-flow], [build flow module, @<:@default=yes@:>@]),
              [build_flow=${enableval}],
              [build_flow="yes"])

AC_ARG_ENABLE(alarm,
              AC_HELP_STRING([--enable-alarm], [build alarm module, @<:@default=yes@:>@]),
              [build_alarm=${enableval}],
              [build_alarm="yes"])

AC_ARG_ENABLE(crypto,
              AC_HELP_STRING([--enable-crypto], [build crypto module, @<:@default=yes@:>@]),
              [build_crypto=${enableval}],
              [build_crypto="yes"])

AC_ARG_ENABLE(tests,
              AC_HELP_STRING([--enable-tests], [build tests, @<:@default=yes@:>@]),
              [build_tests=${enableval}],
              [build_tests="yes"])

AC_ARG_ENABLE(build-samples,
              AC_HELP_STRING([--enable-samples], [build samples, @<:@default=yes@:>@]),
              [build_samples=${enableval}],
              [build_samples="yes"])

AC_ARG_ENABLE(debug,
              AC_HELP_STRING([--enable-debug], [build with debug, @<:@default=no@:>@]),
              [enable_debug=${enableval}],
              [enable_debug="no"])

AC_ARG_ENABLE(lib-install,
              AC_HELP_STRING([--enable-install], [enable installation, @<:@default=yes@:>@]),
              [enable_installation=${enableval}],
              [enable_installation="yes"])


#=====================================================
# Optional packages
#=====================================================

AC_ARG_WITH(event-engin,
            AC_HELP_STRING([--with-event-engin=epoll/libevent/libev], [set event engin, @<:@default=epoll@:>@]),
            [event_engin="$withval"],
            [event_engin="epoll"])

#=====================================================
# optional compilation checks
#=====================================================

AC_MSG_CHECKING([whether CXX supports C++11])
AX_CXX_COMPILE_STDCXX([11], [noext], [mandatory])

AC_MSG_CHECKING([for debug])
if test "x$enable_debug" = "xyes"; then
    CXXFLAGS="$CXXFLAGS -O0 -g"
fi

AM_CONDITIONAL(ENABLE_STATIC, [ test "x$enable_static" = "xyes"])
AM_CONDITIONAL(ENABLE_SHARED, [ test "x$enable_shared" = "xyes"])

AM_CONDITIONAL(BUILD_BASE, [ test "x$build_base" = "xyes"])
AM_CONDITIONAL(BUILD_UTIL, [ test "x$build_util" = "xyes"])
AM_CONDITIONAL(BUILD_EVENT, [ test "x$build_event" = "xyes"])
AM_CONDITIONAL(BUILD_EVENTX, [ test "x$build_eventx" = "xyes"])
AM_CONDITIONAL(BUILD_LOG, [ test "x$build_log" = "xyes"])
AM_CONDITIONAL(BUILD_NETWORK, [ test "x$build_network" = "xyes"])
AM_CONDITIONAL(BUILD_HTTP, [ test "x$build_http" = "xyes"])
AM_CONDITIONAL(BUILD_COROUTINE, [ test "x$build_coroutine" = "xyes"])
AM_CONDITIONAL(BUILD_MQTT, [ test "x$build_mqtt" = "xyes"])
AM_CONDITIONAL(BUILD_TERMINAL, [ test "x$build_terminal" = "xyes"])
AM_CONDITIONAL(BUILD_MAIN, [ test "x$build_main" = "xyes"])
AM_CONDITIONAL(BUILD_FLOW, [ test "x$build_flow" = "xyes"])
AM_CONDITIONAL(BUILD_ALARM, [ test "x$build_alarm" = "xyes"])
AM_CONDITIONAL(BUILD_CRYPTO, [ test "x$build_crypto" = "xyes"])
AM_CONDITIONAL(BUILD_TESTS, [test "x$build_tests" = "xyes"])
AM_CONDITIONAL(BUILD_SAMPLES, [test "x$build_samples" = "xyes"])

AM_CONDITIONAL(ENABLE_INSTALLATION, [test "x$enable_installation" = "xyes"])
AM_CONDITIONAL(ENABLE_LIBEVENT, [test "x$event_engin" = "xlibevent"])
AM_CONDITIONAL(ENABLE_LIBEV, [test "x$event_engin" = "xlibev"])
AM_CONDITIONAL(ENABLE_EPOLL, [test "x$event_engin" = "xepoll"])
AM_CONDITIONAL(ENABLE_DEBUG, [test "x$enable_debug" = "xyes"])


AC_SUBST([CPPFLAGS])
AC_SUBST([CXXFLAGS])
AC_SUBST([CFLAGS])
AC_SUBST([LDFLAGS])


AC_CONFIG_FILES([
Makefile
tbox/Makefile
tbox/base/Makefile
tbox/event/Makefile
tbox/eventx/Makefile
tbox/alarm/Makefile
tbox/coroutine/Makefile
tbox/crypto/Makefile
tbox/flow/Makefile
tbox/http/Makefile
tbox/log/Makefile
tbox/main/Makefile
tbox/network/Makefile
tbox/terminal/Makefile
tbox/util/Makefile
tbox/mqtt/Makefile
tbox/tests/Makefile
tbox/samples/Makefile
])

AC_OUTPUT

AC_MSG_NOTICE([

    Configuration for cpp-tbox complete:

        Source code location:               ${srcdir}/tbox
        C++ Compiler:                       ${CXX}
        C++ Compiler flags:                 ${CXXFLAGS}
        C/C++ Preprocessor flags:           ${CPPFLAGS}
        C++ Linker flags:                   ${LDFLAGS}

        static:                             ${enable_static}
        shared:                             ${enable_shared}

        build base:                         ${build_base}
        build util:                         ${build_util}
        build event:                        ${build_event}
        build eventx:                       ${build_eventx}
        build log:                          ${build_log}
        build network:                      ${build_network}
        build http:                         ${build_http}
        build coroutine:                    ${build_coroutine}
        build mqtt:                         ${build_mqtt}
        build terminal:                     ${build_terminal}
        build main:                         ${build_main}
        build flow:                         ${build_flow}
        build alarm:                        ${build_alarm}
        build crypto:                       ${build_crypto}
        build tests:                        ${build_tests}
        build samples:                      ${build_samples}

        event engin:                        ${event_engin}
        debug:                              ${enable_debug}
])

