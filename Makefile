#--------------------------------------------------------------------------
# $Id: Makefile,v 1.2 1996/01/16 16:29:15 Madsen Exp $
#--------------------------------------------------------------------------
# Visual Binary Diff
# Copyright 1995 by Christopher J. Madsen

CC=gcc
CXX=gcc
SHELL=cmd.exe

CFLAGS=-Wall
CXXFLAGS=$(CFLAGS)
LDFLAGS=-Zomf -Zcrtdll

OBJECTS=vbindiff.obj getopt.obj getopt1.obj

%.obj: %.c
	$(CC) -c -Zomf $(CPPFLAGS) $(CFLAGS) $< -o $@

%.obj: %.cc
	$(CC) -c -Zomf $(CPPFLAGS) $(CXXFLAGS) $< -o $@

all: vbindiff.exe

vbindiff.exe: $(OBJECTS)
	$(CC) $(LDFLAGS) -o vbindiff.exe vbindiff.def \
		$(OBJECTS) -lvideo -lstdcpp -s

vbindiff.obj: getopt.h

getopt.obj: getopt.h

getopt1.obj: getopt.h

clean:
	del *.obj

dist: all
	@echo Did you remember to update File_ID.DIZ?
	-del vbindiff.zip source.zip
	zip -9 source *.cc *.c *.h *.def Makefile
	zip -9 vbindiff *.1st *.diz *.exe
	@zip -9j vbindiff /emx/doc/COPYING
	@zip -0 vbindiff source.zip
	-@del source.zip
