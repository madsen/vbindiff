## Process this file with automake to produce Makefile.in
#---------------------------------------------------------------------
# Makefile.am
# Copyright 2008 Christopher J. Madsen
#
# Automake makefile for VBinDiff
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#--------------------------------------------------------------------

bin_PROGRAMS = vbindiff
dist_man_MANS = vbindiff.1
vbindiff_SOURCES = vbindiff.cpp tables.h curses/ConWin.cpp curses/ConWin.hpp  \
		   curses/FileIO.hpp GetOpt/GetOpt.cpp GetOpt/GetOpt.hpp
EXTRA_DIST = \
	cjm-style.el		\
	putty.src		\
	README.PuTTY		\
	tools/vbindiff.pod.tt	\
	tools/NEWS.tt		\
	tools/ReadMe.tt		\
	tools/configure.commit	\
	tools/genfile.pl	\
	tools/getversion.pm	\
	tools/maketxt.pl	\
	tools/version.h.tt	\
	tools/zipdist.pl	\
	win32/ConWin.cpp	\
	win32/ConWin.hpp	\
	win32/FileIO.hpp	\
	win32/StdAfx.cpp	\
	win32/config.h		\
	win32/vbindiff.dsp	\
	win32/vbindiff.dsw	\
	win32/vbindiff.rc	\
	win32/version.h

AM_CPPFLAGS = -I$(srcdir)/curses

AM_CFLAGS = -Wall -D_FILE_OFFSET_BITS=64
AM_CXXFLAGS = $(AM_CFLAGS)

GENFILE = perl tools/genfile.pl

README : tools/ReadMe.tt configure.ac
	$(GENFILE) $<

NEWS : tools/NEWS.tt configure.ac
	$(GENFILE) $<

vbindiff.1 : tools/vbindiff.pod.tt configure.ac
	$(GENFILE) $<

win32/version.h : tools/version.h.tt configure.ac
	$(GENFILE) $<
