#!/bin/sh

gcc -Wall -O2 -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 -I/usr/lib/jvm/java-6-sun/include -I/usr/lib/jvm/java-6-sun/include/linux/ -I /home/karen/Dev/Grid/hdfs/trunk/src/c++/libhdfs/ -I /home/karen/Dev/Grid/hdfs/fuse/install/include/ -I ../include/ -L /home/karen/Dev/Grid/hdfs/fuse/install/lib/ -lfuse -L /home/karen/Dev/Grid/hdfs/hadoop/install/libhdfs/ -lhdfs  -L /usr/lib/jvm/java-6-sun/jre/lib/i386/server -ljvm -o fuse hdfs.c -L ../lib/ -lhdfs-fuse

if [ "$HADOOP_HOME" = "" ]; then
 HADOOP_HOME=/usr/local/share/hadoop
fi

for f in ls $HADOOP_HOME/lib/*.jar $HADOOP_HOME/*.jar ; do
  CLASSPATH=$CLASSPATH:$f
done

JAVA_HOME=
HADOOP_HOME=
FUSE_HOME=
HDFS_FUSE_HOME=
OS=linux
MARCH=i386

HADOOP_LIBHDFS_INCLUDE=$HADOOP_HOME/src/c++/libhdfs/
JAVA__INCLUDE=$JAVA_HOME/include:$JAVA_HOME/include/$OS
FUSE_INCLUDE=$FUSE_HOME/include
HDFS_FUSE_LIB_DIR=$HDFS_FUSE_HOME/include

HADOOP_LIBHDFS_DIR=$HADOOP_HOME/libhdfs -lhdfs
JAVA_JVM_DIR=$JAVA_HOME/jre/lib/$MARCH/server -ljvm
FUSE_LIB_DIR=$FUSE_HOME/lib -lfuse
HDFS_FUSE_LIB_DIR=$HDFS_FUSE_HOME/lib -lhdfs-fuse

HDFS_FUSE_CONF=$HDFS_FUSE_HOME/conf

LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HADOOP_LIBHDFS_DIR:$JAVA_JVM_DIR:$FUSE_LIB_DIR:$HDFS_FUSE_LIB_DIR