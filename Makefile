#--------------------------------------------------------------------------
# $Id: Makefile,v 1.4 1996/01/18 16:53:43 Madsen Exp $
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

.PHONY: all clean dist

vbindiff.exe: $(OBJECTS)
	$(CC) $(LDFLAGS) -o vbindiff.exe vbindiff.def \
		$(OBJECTS) -lvideo -lstdcpp

$(OBJECTS): getopt.h

clean:
	$(RM) *.o *.obj

dist: all
	@echo Did you remember to update File_ID.DIZ, \
		ReadMe.1st, and VBinDiff.txt?
	-$(RM) vbindiff.zip source.zip
	zip -9 source *.cc *.c *.h *.def Makefile
	zip -9 vbindiff *.1st *.diz *.exe VBinDiff.txt
	@zip -9j vbindiff /emx/doc/COPYING
	@zip -0 vbindiff source.zip
	-@$(RM) source.zip
