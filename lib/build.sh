#!/bin/sh

JAVA_HOME=
FUSE_HOME=
HADOOP_HOME=

gcc -Wall -O2 -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -I ${JAVA_HOME}/include/ -I ${JAVA_HOME}/include/linux/ -I ${HADOOP_HOME}/src/c++/libhdfs/ -I ${FUSE_HOME}/include/ -I ../include/ -fPIC -shared -o libhdfs-fuse.so *.c


