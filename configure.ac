AC_INIT([broker], [0.0.1], [stumathews@gmail.com])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h:config.hin)
AC_PROG_CC
AC_PROG_CC_STDC
AC_C_CONST
AC_SUBST([LIBINC], ["-I ../../stulibc/src/includes -I ../common -I ../server -I ../../msgpack-c/include -I ../../sqlite/sqlite-autoconf-3140100"])
AC_SUBST([LIBLINK],["-L/usr/local/lib -lmsgpackc -lstu -lsqlite3"])
#AC_LIBTOOL_WIN32_DLL
AC_CHECK_HEADERS([sys/socket.h])
AC_CHECK_HEADERS([winsock2.h])
AC_CHECK_HEADERS([windows.h])
AC_PROG_LIBTOOL
AM_INIT_AUTOMAKE([subdir-objects])
AC_ARG_WITH(inspect,
                                                                    [  --with-inspect           Inspect test suite errors],
                                                                                [ AC_DEFINE(INSPECT_ERRORS, 1, "Inspect test suite errors")],
                                                                                [ AC_DEFINE(INSPECT_ERRORS, 0, "Inspect test suite errors")])
AC_OUTPUT(Makefile broker/Makefile client/Makefile server/Makefile)
AC_PATH_PROG([PERL], [perl])
