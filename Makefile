#--------------------------------------------------------------------------
# $Id: Makefile,v 1.5 1996/01/18 18:06:16 Madsen Exp $
#--------------------------------------------------------------------------
# Visual Binary Diff
# Copyright 1995 by Christopher J. Madsen
#
# Makefile for use with GNU Make, EMX 0.9b, and GCC 2.7.2

CC=gcc
CXX=gcc
RM=del
SHELL=cmd.exe

CFLAGS=-Wall -O3
CXXFLAGS=$(CFLAGS)
LDFLAGS=-Zomf -Zcrtdll -s

OBJECTS=vbindiff.obj getopt.obj getopt1.obj

ifdef DEBUG
# Use a.out format so we can use GDB:
OBJECTS:=$(subst .obj,.o,$(OBJECTS))
CFLAGS:=$(CFLAGS) -O0 -g
LDFLAGS=-g
endif

%.obj: %.c
	$(CC) -c -Zomf $(CPPFLAGS) $(CFLAGS) $< -o $@

%.obj: %.cc
	$(CXX) -c -Zomf $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all: vbindiff.exe

.PHONY: all clean dist distclean mostlyclean maintainer-clean

vbindiff.exe: $(OBJECTS)
	$(CC) $(LDFLAGS) -o vbindiff.exe vbindiff.def \
		$(OBJECTS) -lvideo -lstdcpp

$(OBJECTS): getopt.h

clean distclean mostlyclean:
	$(RM) *.o *.obj

maintainer-clean:
	$(RM) *.o *.obj *.exe

# VBD_TMP is used only by `make dist'; you shouldn't need to change it:
VBD_TMP=$(subst /,\\,$(TMP))\\vbindiff

dist: all
	@echo Did you remember to update File_ID.DIZ, \
		ReadMe.1st, and VBinDiff.txt?
	-$(RM) vbindiff.zip source.zip
	@mkdir $(VBD_TMP)
	@copy *.cc $(VBD_TMP)
	@copy *.c $(VBD_TMP)
	@copy *.h $(VBD_TMP)
	@copy *.def $(VBD_TMP)
	copy Makefile $(VBD_TMP)\\Makefile
	copy VBinDiff.txt $(VBD_TMP)\\VBinDiff.txt
	attrib -r $(VBD_TMP)\\*
	zip -9mj source $(VBD_TMP)\\*
	copy ReadMe.1st $(VBD_TMP)\\ReadMe.1st
	copy File_ID.DIZ $(VBD_TMP)\\File_ID.DIZ
	@copy *.exe $(VBD_TMP)
	attrib -r $(VBD_TMP)\\*
	zip -9mj vbindiff $(VBD_TMP)\\*
	@zip -9j vbindiff /emx/doc/COPYING
	@zip -0m vbindiff source.zip
	@rmdir $(VBD_TMP)
