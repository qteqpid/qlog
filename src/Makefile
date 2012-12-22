# Hiredis Makefile
# Copyright (C) 2010-2011 Salvatore Sanfilippo <antirez at gmail dot com>
# Copyright (C) 2010-2011 Pieter Noordhuis <pcnoordhuis at gmail dot com>
# This file is released under the BSD license, see the COPYING file

OBJ=net.o hiredis.o sds.o qstring.o dso.o dict.o mymalloc.o qsignal.o
BINS=qlog-server
LIBNAME=libqlog

# Fallback to gcc when $CC is not in $PATH.
#CC:=$(shell sh -c 'type $(CC) >/dev/null 2>/dev/null && echo $(CC) || echo gcc')
CC:=gcc
OPTIMIZATION?=-O2
WARNINGS=-Wall -W -Wstrict-prototypes -Wwrite-strings
REAL_CFLAGS=$(OPTIMIZATION) -fPIC $(CFLAGS) $(WARNINGS) $(DEBUG) $(PROF) $(ARCH)
REAL_LDFLAGS=$(LDFLAGS) $(PROF) $(ARCH) -ldl

STLIBSUFFIX=a
STLIBNAME=$(LIBNAME).$(STLIBSUFFIX)
STLIB_MAKE_CMD=ar rcs $(STLIBNAME)

all: $(BINS)

# Deps (use make dep to generate this)
net.o: net.c fmacros.h net.h hiredis.h
server.o: server.c hiredis.h qstring.h dso.h dict.h mymalloc.h qsignal.h
hiredis.o: hiredis.c fmacros.h hiredis.h net.h sds.h
sds.o: sds.c sds.h
qstring.o: qstring.c mymalloc.h
dso.o: dso.c
dict.o: dict.c
mymalloc.o:mymalloc.c
qsignal.o:qsignal.c

$(STLIBNAME): $(OBJ)
	$(STLIB_MAKE_CMD) $(OBJ)

qlog-%: %.o $(STLIBNAME)
	$(CC) -o $@ $(REAL_LDFLAGS) $< $(STLIBNAME)

.c.o:
	$(CC) -std=gnu99 -pedantic -c $(REAL_CFLAGS) $<

clean:
	rm -rf $(STLIBNAME) $(BINS) *.o 


# Installations #
PREFIX:=/etc/qlog
CONFDIR:=$(PREFIX)/conf
CHANNELDIR:=$(PREFIX)/channels

install: $(DYLIBNAME) $(STLIBNAME)
	@if [ ! -d $(CONFDIR) ]; then mkdir -p $(CONFDIR); fi
	@if [ ! -d $(CHANNELDIR) ]; then mkdir -p $(CHANNELDIR); fi
	@if [ ! -f $(CONFDIR)/qlog.conf ]; then cp -a ../qlog.conf $(CONFDIR)/qlog.conf; else cp -a ../qlog.conf $(CONFDIR)/qlog.conf.new; fi


32bit:
	@echo ""
	@echo "WARNING: if this fails under Linux you probably need to install libc6-dev-i386"
	@echo ""
	$(MAKE) CFLAGS="-m32" LDFLAGS="-m32"

debug:
	$(MAKE) OPTIMIZATION="" DEBUG="-g -ggdb -g3"

gprof:
	$(MAKE) PROF="-pg" 

.PHONY: all clean install 32bit debug gprof
