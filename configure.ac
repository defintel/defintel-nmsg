AC_PREREQ(2.60)
AC_INIT([defintel-nmsg], [0.5])
AM_INIT_AUTOMAKE([1.10 foreign])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_FILES([Makefile example/Makefile])
AC_CONFIG_HEADER([config.h])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LIBTOOL

m4_sinclude(check_libnmsg.m4)

AC_OUTPUT
