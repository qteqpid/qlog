# Hiredis Makefile
# Copyright (C) 2010-2011 Salvatore Sanfilippo <antirez at gmail dot com>
# Copyright (C) 2010-2011 Pieter Noordhuis <pcnoordhuis at gmail dot com>
# This file is released under the BSD license, see the COPYING file

QLOG_MAJOR=0
QLOG_MINOR=1
DYLIBSUFFIX=so
DYLIBMAJORSUFFIX=$(QLOG_MAJOR)
DYLIBMINORSUFFIX=$(QLOG_MAJOR).$(QLOG_MINOR)

SOURCES=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SOURCES))
DYLIBNAMES=$(patsubst %.c,%.$(DYLIBSUFFIX),$(SOURCES))


# Fallback to gcc when $CC is not in $PATH.
CC:=$(shell sh -c 'type $(CC) >/dev/null 2>/dev/null && echo $(CC) || echo gcc')
OPTIMIZATION?=-O2
WARNINGS=-Wall -W -Wstrict-prototypes -Wwrite-strings
CFLAGS=-I../src
REAL_CFLAGS=$(OPTIMIZATION) -fPIC $(CFLAGS) $(ARCH) $(WARNINGS) $(DEBUG) $(PROF)
REAL_LDFLAGS=$(LDFLAGS) $(ARCH) $(PROF) -ldl


all: $(DYLIBNAMES) 

$(DYLIBNAMES): $(OBJS)
	$(CC) -shared -Wl,-soname,$@.$(DYLIBMINORSUFFIX) -o $@ $(LDFLAGS) $<


.c.o:
	$(CC) -std=c99 -pedantic -c $(REAL_CFLAGS) $<

clean:
	rm -rf $(DYLIBNAMES) $(OBJS)


# Installation related variables and target
PREFIX:=/etc/qlog
CHANNELDIR=$(PREFIX)/channels

ifeq ($(uname_S),SunOS)
  INSTALL?= cp -r
endif
INSTALL?= cp -af

install: $(DYLIBNAMES) 
	@mkdir -p $(CHANNELDIR)
	$(INSTALL) $< $(CHANNELDIR)

32bit:
	@echo ""
	@echo "WARNING: if this fails under Linux you probably need to install libc6-dev-i386"
	@echo ""
	$(MAKE) ARCH="-m32"

debug:
	$(MAKE) OPTIMIZATION="" DEBUG="-g -ggdb -g3"

gprof:
	$(MAKE) PROF="-pg" 

.PHONY: all clean 32bit debug gprof
