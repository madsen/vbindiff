## Process this file with automake to produce Makefile.in
# $Id: Makefile.am 4620 2005-03-25 19:59:42Z cjm $

bin_PROGRAMS = vbindiff
dist_man_MANS = vbindiff.1
vbindiff_SOURCES = vbindiff.cpp tables.h curses/ConWin.cpp curses/ConWin.hpp  \
		   curses/FileIO.hpp GetOpt/GetOpt.cpp GetOpt/GetOpt.hpp
EXTRA_DIST = vbindiff.pod cjm-style.el putty.src \
	win32/ConWin.cpp	\
	win32/ConWin.hpp	\
	win32/FileIO.hpp	\
	win32/StdAfx.cpp	\
	win32/config.h		\
	win32/update.pl		\
	win32/vbindiff.dsp	\
	win32/vbindiff.dsw

AM_CPPFLAGS = -I$(srcdir)/curses

AM_CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
AM_CXXFLAGS = $(AM_CFLAGS)

vbindiff.1 : vbindiff.pod
	pod2man --center="Christopher J. Madsen" \
		--release="VBinDiff $(PACKAGE_VERSION)" $< >$@
