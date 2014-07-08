#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(zasm, 0.1a, James Rubingh <james@wrive.com>)
AC_CONFIG_SRCDIR([src/zasm.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_ARG_WITH(cc,
[  --with-cc=xxx           use XXX as the C compiler],
CC=$with_cc)

AC_LANG(C)
AC_C_CONST
AC_C_INLINE

AC_PROG_CC

AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

#create some output makefiles yo
AC_OUTPUT(Makefile src/Makefile doc/Makefile lib/Makefile)
