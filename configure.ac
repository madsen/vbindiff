#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# $Id: configure.ac 4592 2005-03-12 17:11:36Z cjm $

AC_PREREQ(2.59)
AC_INIT([[Visual Binary Diff]], [[3.0]], [[cjm@pobox.com]], [[vbindiff]])
AM_INIT_AUTOMAKE([1.9 foreign])
AC_CONFIG_SRCDIR([vbindiff.cpp])
AC_CONFIG_HEADER([config.h])
AC_LANG([C++])

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
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_SETVBUF_REVERSED
AC_FUNC_STRTOD
AC_CHECK_FUNCS([memset strchr strrchr strtol strtoul])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
