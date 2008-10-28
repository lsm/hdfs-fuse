/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * hdfs-fuse.h
 * Copyright (C) Peng Zhao 2008 <jass.zhao@gmail.com>
 * 
 * hdfs-fuse.h is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * main.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with main.c.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */
 
#ifndef _HDFS_FUSE_H_
#define _HDFS_FUSE_H_

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <math.h>
#include <dirent.h>
#include <sys/time.h>
#include <pthread.h>

#ifdef _XATTR_
#include <sys/attr.h>
#endif

#include <fuse.h>
#include <fuse_opt.h>

#include <hdfs.h>

#define MAX_FILENAME_LENGTH	1024
#define MAX_HOSTNAME_LENGTH	1024

typedef enum Log_Level {
	LOG_FATAL,
	LOG_CRITICAL,
	LOG_ERROR,
	LOG_WARNING,
	LOG_NOTICE,
	LOG_INFO,
	LOG_DEBUG,
	LOG_TRACE
} Log_Level;

static char *log_level_array[8] = {
	"LOG_FATAL",
	"LOG_CRITICAL",
	"LOG_ERROR",
	"LOG_WARINING",
	"LOG_NOTICE",
	"LOG_INFO",
	"LOG_DEBUG",
	"LOG_TRACE"
};

typedef struct KeyValue {
    char *key;
    char *value;
} KeyValue;

typedef struct {
	int cfgc;
	KeyValue *cfgv;
	char *logdir;
	Log_Level loglv;
	char *hostname;
	int port;
} Config;

typedef struct {
	char *cfgdir;
} Env;

Config config;
Env env;

#endif	//	_HDFS_FUS_H_

