#!/bin/sh

JAVA_HOME=
FUSE_HOME=
HADOOP_HOME=

gcc -Wall -O2 -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -I ${JAVA_HOME}/include/ -I ${JAVA_HOME}/include/linux/ -I ${HADOOP_HOME}/src/c++/libhdfs -I ${FUSE_HOME}/include/ -I ../include/ -L ${FUSE_HOME}/lib/ -lfuse -L ${HADOOP_HOME}/libhdfs -lhdfs  -L ${JAVA_HOME}/jre/lib/i386/server/ -ljvm -o hdfs-fuse hdfs.c -L ../lib/ -lhdfs-fuse

