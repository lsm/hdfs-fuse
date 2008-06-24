# -*- Mode: Makefile; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
#
# Makefile
# Copyright (C) Peng Zhao 2008 <jass.zhao@gmail.com>
# 
# Makefile is free software.
# 
# You may redistribute it and/or modify it under the terms of the
# GNU General Public License, as published by the Free Software
# Foundation; either version 2 of the License, or (at your option)
# any later version.
# 
# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with main.c.  If not, write to:
# 	The Free Software Foundation, Inc.,
# 	51 Franklin Street, Fifth Floor
# 	Boston, MA  02110-1301, USA.
#

CC = gcc
CFLAGS = -Wall -O2 -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26
OBJECTS = libhdfs-fuse.o
INCFLAGS = -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux/ -I$(HADOOP_HOME)/src/c++/libhdfs  -I $(FUSE_HOME)/include -I ../include/
LDFLAGS = -Wl,-rpath,/usr/local/lib 
LIBS =

all: libhdfs-fuse.so

libhdfs-fuse.so: $(OBJECTS)
	$(CC) -fPIC -shared -o libhdfs-fuse.so $(OBJECTS) $(LDFLAGS) $(LIBS)

.SUFFIXES:
.SUFFIXES:	.c .cc .C .cpp .o

*.o:*.c
	$(CC) -o $@ -c $(CFLAGS) $< $(INCFLAGS)

count:
	wc *.c *.cc *.C *.cpp *.h *.hpp

clean:
	rm -f *.o *.so

.PHONY: all
.PHONY: count
.PHONY: clean

