## Process this file with automake to produce Makefile.in
# $Id: Makefile.am 4605 2005-03-21 17:56:26Z cjm $

bin_PROGRAMS = vbindiff
dist_man_MANS = vbindiff.1
vbindiff_SOURCES = vbindiff.cpp curses/ConWin.cpp curses/ConWin.hpp  \
		   GetOpt/GetOpt.cpp GetOpt/GetOpt.hpp
EXTRA_DIST = vbindiff.pod

AM_CFLAGS = -Wall -D_FILE_OFFSET_BITS=64 -Icurses

AM_CXXFLAGS = $(AM_CFLAGS)

vbindiff.1 : vbindiff.pod
	pod2man --center="Christopher J. Madsen" \
		--release="VBinDiff $(PACKAGE_VERSION)" $< >$@
