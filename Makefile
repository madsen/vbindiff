#--------------------------------------------------------------------------
# $Id: Makefile,v 1.1 1996/01/16 02:07:12 Madsen Exp $
#--------------------------------------------------------------------------
# Visual Binary DIFF
# Copyright 1995 by Christopher J. Madsen

CC=gcc
CXX=gcc
SHELL=cmd.exe

CFLAGS=-Wall
CXXFLAGS=$(CFLAGS)

OBJECTS=vbindiff.o getopt.o getopt1.o

all: vbindiff.exe

vbindiff.exe: $(OBJECTS)
	$(CC) $(LDFLAGS) -o vbindiff.exe $(OBJECTS) -lvideo -lstdcpp -s

vbindiff.o: getopt.h

getopt.o: getopt.h

getopt1.o: getopt.h

clean:
	del *.o

dist: all
	@echo Did you remember to update File_ID.DIZ?
	-del vbindiff.zip source.zip
	zip -9 source.zip *.cc *.c *.h Makefile
	zip -9 vbindiff *.1st *.diz *.exe
	@zip -9j vbindiff /emx/doc/COPYING
	@zip -0 vbindiff source.zip
	-@del source.zip
