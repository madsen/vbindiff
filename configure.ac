#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# $Id: configure.ac 4621 2005-03-25 20:21:26Z cjm $

AC_PREREQ(2.59)
AC_INIT([[Visual Binary Diff]], [[3.0_beta1]], [[cjm@pobox.com]], [[vbindiff]])
AM_INIT_AUTOMAKE([1.9 foreign])
AC_CONFIG_SRCDIR([vbindiff.cpp])
AC_CONFIG_HEADER([config.h])
AC_LANG([C++])
AC_REVISION($Revision: 4621 $)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([ncurses], [initscr], ,
             [AC_MSG_ERROR([The ncurses library is required])])
AC_CHECK_LIB([panel], [new_panel], ,
             [AC_MSG_ERROR([The panel library is required])])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([atexit memset strchr strerror strrchr strtoul])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
