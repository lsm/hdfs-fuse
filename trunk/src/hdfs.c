/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * hdfs-fuse.c
 * Copyright (C) Peng Zhao 2008 <jass.zhao@gmail.com>
 * 
 * hdfs-fuse.c is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * The file is distributed in the hope that it will be useful,
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

#include <hdfs-fuse.h>

extern int fuse_config_init(const char * cfgdir, const char * exe, KeyValue * cfgv, int cfgc);
extern int fuse_log_init(const char * path, const char * exename, Log_Level level);
extern void fuse_log(Log_Level level, const char * fname, const char * msg,...);

void *hdfs_init(struct fuse_conn_info *conn) 
{
	static char fname[] = "hdfs_init";
	hdfsFS *hdfs = (hdfsFS*)malloc(sizeof(hdfsFS));
	
	memset(hdfs, 0, sizeof(hdfsFS));
	*hdfs = hdfsConnect(config.hostname, config.port);
	if (*hdfs == NULL) {
		fuse_log(LOG_DEBUG, fname, "Cannot connect to HDFS. Hostname: %s, port: %d", config.hostname, config.port);
	}
	return (void*)hdfs;
}

void hdfs_destroy(void *arg) 
{
	hdfsFS *hdfs = (hdfsFS*)arg;

	hdfsDisconnect(*hdfs);
	free(hdfs);			// should we free or let fuse cares for us ?
	
	return;
}

int hdfs_open (const char *path, struct fuse_file_info *info)  
{
	static char fname[] = "hdfs_open";
	hdfsFS hdfs;
	hdfsFile file;

	if (!path) {
		fuse_log(LOG_DEBUG, fname, "Missing arguments");
		return -EINVAL;
	}	
	memset(info, 0, sizeof(struct fuse_file_info));
	hdfs = hdfsConnect(config.hostname, config.port);
	if (hdfs == NULL) {
		fuse_log(LOG_DEBUG, fname, "Cannot connect to HDFS. Hostname: %s, port: %d", config.hostname, config.port);
		return -EIO;
	} 

	if ((file = hdfsOpenFile(hdfs, path, info->flags, 0, 0, 0)) == NULL) {	// use defaut settings
		fuse_log(LOG_DEBUG, fname, "Cannot open the file %s", path);
		return -EIO;
	} 		
	info->fh = (uint64_t)file;

	return 0;
}

int hdfs_read (const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *info)
{
	static char fname[] = "hdfs_read";
	hdfsFS hdfs;
	hdfsFile file;

	if (!path || !buf) {
		fuse_log(LOG_DEBUG, fname, "Missing arguments");
		return -EINVAL;
	}		
	hdfs = hdfsConnect(config.hostname, config.port);
	if (hdfs == NULL) {
		fuse_log(LOG_DEBUG, fname, "Cannot connect to HDFS. Hostname: %s, port: %d", config.hostname, config.port);
		return -EIO;
	}
	file = (hdfsFile)info->fh;
	tSize count = hdfsPread(hdfs, file, (tOffset)offset, (void*)buf, (tSize)size);
	if (count<0) {
		fuse_log(LOG_DEBUG, fname, "Failed to read from the file %s", path);
	}

	return (int)count;
}

int hdfs_create(const char *path, mode_t mode, struct fuse_file_info *info)
{
	static char fname[] = "hdfs_create";
	hdfsFS hdfs;
	hdfsFile file;

	if (!path) {
		fuse_log(LOG_DEBUG, fname, "Missing arguments");
		return -EINVAL;
	}	
	memset(info, 0, sizeof(struct fuse_file_info));
	hdfs = hdfsConnect(config.hostname, config.port);
	if (hdfs == NULL) {
		fuse_log(LOG_DEBUG, fname, "Cannot connect to HDFS. Hostname: %s, port: %d", config.hostname, config.port);
		return -EIO;
	} 

	if ((file = hdfsOpenFile(hdfs, path, O_WRONLY, 0, 0, 0)) == NULL) {	// use defaut settings
		fuse_log(LOG_DEBUG, fname, "Cannot create the file %s", path);
		return -EIO;
	} 		
	info->fh = (uint64_t)file;

	return 0;	
}


int hdfs_write (const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *info)
{
	static char fname[] = "hdfs_write";
	hdfsFS hdfs;
	hdfsFile file;

	if (!path || !buf) {
		fuse_log(LOG_DEBUG, fname, "Missing arguments");
		return -EINVAL;
	}	
	hdfs = hdfsConnect(config.hostname, config.port);
	if (hdfs == NULL) {
		fuse_log(LOG_DEBUG, "hdfs_write", "Cannot connect to HDFS. Hostname: %s, port: %d", config.hostname, config.port);
		return -EIO;
	}
	file = (hdfsFile)info->fh;
	
	if (hdfsSeek(hdfs, file, (tOffset)offset) < 0) {
		fuse_log(LOG_DEBUG, fname, "Invalid offset");
		return -ESPIPE;
	}
	tSize count =  hdfsWrite(hdfs, file, (void*)buf, (tSize)size);
	if (count < 0) {
		fuse_log(LOG_DEBUG, fname, "Failed to write to the file %s", path);
	}
#ifdef _HDFS_WRITE_FLUSH
	if (hdfsFlush(hdfs, file) < 0) {
		fuse_log(LOG_DEBUG, fname, "Failed to flush the write buffer to the file %s", path);
	}
#endif

	return count;
}

int hdfs_rename(const char *src, const char *dest)
{
	static char fname[] = "hdfs_rename";
	hdfsFS hdfs;

	if (src == NULL || dest  == NULL) {
		fuse_log(LOG_DEBUG, fname, "Missing arguments");
		return -EINVAL;
	}
	if (!strcmp(src, dest)) {
		/*	src/dest are the same, we simply ignore this call	*/
		fuse_log(LOG_WARNING, fname, "The source/destination path are the same.");
		return 0;
	}

	hdfs = hdfsConnect(config.hostname, config.port);
	if (hdfs == NULL) {
		fuse_log(LOG_DEBUG, "hdfs_open", "Cannot connect to HDFS. Hostname: %s, port: %d", config.hostname, config.port);
		return -EIO;
	}
	int ret = hdfsRename(hdfs, src, dest);
	if (ret < 0) {
		fuse_log(LOG_DEBUG, fname, "Cannot mv the file from %s to %s", src, dest);
	}
	
	return ret;
}

int hdfs_unlink(const char *path)
{
	static char fname[] = "hdfs_unlink";
	hdfsFS hdfs;

	if (!path) {
		fuse_log(LOG_DEBUG, fname, "Missing argument");
		return -EINVAL;
	}
	
	hdfs = hdfsConnect(config.hostname, config.port);
	if (hdfs == NULL) {
		fuse_log(LOG_DEBUG, "hdfs_open", "Cannot connect to HDFS. Hostname: %s, port: %d", config.hostname, config.port);
		return -EIO;
	}
	int ret = hdfsDelete(hdfs, path);
	if (ret < 0) {
		fuse_log(LOG_DEBUG, fname, "Cannot delete the file %s", path);
	}
	return ret;
}			

int hdfs_release (const char *path, struct fuse_file_info *info)    
{
	static char fname[] = "hdfs_release";
	hdfsFS hdfs;
	hdfsFile file;

	if (!path) {
		fuse_log(LOG_DEBUG, fname, "Missing arguments");
		return -EINVAL;
	}	
	hdfs = hdfsConnect(config.hostname, config.port);
	if (hdfs == NULL) {
		fuse_log(LOG_DEBUG, "hdfs_open", "Cannot connect to HDFS. Hostname: %s, port: %d", config.hostname, config.port);
		return -EIO;
	}
	file = (hdfsFile)info->fh;

	int ret = hdfsCloseFile(hdfs, file);
	if (ret < 0) {
		fuse_log(LOG_DEBUG, fname, "Cannot close the file %s", path);
	}
	
	memset(info, 0, sizeof(struct fuse_file_info));
	
	return ret;
}

int hdfs_getattr(const char *path, struct stat *st)
{
	static char fname[] = "hdfs_getattr";
	hdfsFS hdfs;
	hdfsFileInfo *info;

	if (path == NULL) {
		fuse_log(LOG_DEBUG, fname, "Missing arguments");
		return -EINVAL;
	}	
	hdfs = hdfsConnect(config.hostname, config.port);
	if (hdfs == NULL) {
		fuse_log(LOG_DEBUG, fname, "Cannot connect to HDFS. Hostname: %s, port: %d", config.hostname, config.port);
		return -EIO;
	}
	if (hdfsExists(hdfs, path) < 0) {
		fuse_log(LOG_DEBUG, fname, "There is no such file or directory %s", path);
		return -ENOENT;
	}
	info = hdfsGetPathInfo(hdfs, path);
	if (info == NULL) {
		fuse_log(LOG_DEBUG, fname, "Cannot get the information of the file %s", path);
		return -EIO;		// the file does NOT exist
	}
	memset(st, 0, sizeof(struct stat));
	st->st_mode 		= (info->mKind == kObjectKindDirectory)?S_IFDIR|0755:S_IFREG|0444;
	st->st_size		= (info->mKind == kObjectKindDirectory)?4096:info->mSize;
	st->st_nlink 		= 1;
	st->st_atime    	= info->mLastMod;  
	st->st_ctime    	= info->mLastMod;
	st->st_mtime    	= info->mLastMod;  
	st->st_blksize  	= (blksize_t)info->mBlockSize;  
	//st->st_blocks   	=  ceil(st->st_size/st->st_blksize);
	//st->st_uid      	= default_id;  
	//st->st_gid      	= default_id; 	 	

	hdfsFreeFileInfo(info, 1);
    	return 0;
}

/*

typedef struct  {     
	tObjectKind mKind;   	// file or directory        
	char *mName;         	//  the name of the file 
	tTime mLastMod;   		// the last modification time for the file
	tOffset mSize;       		// the size of the file in bytes 
	short mReplication;    	// the count of replicas   
	tOffset mBlockSize;  	// the block size for the file 
} hdfsFileInfo;

*/

int hdfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *info)
{  
	static char fname[] = "hdfs_readdir";
	(void) offset;  
	(void) info;  
	hdfsFS hdfs;
	hdfsFileInfo *fileinfo;
	static int notfirsttime = 0;
	static int off = 0;

	if (!notfirsttime) {
		char url[MAX_FILENAME_LENGTH];
		memset(url, 0, MAX_FILENAME_LENGTH);
		sprintf(url, "hdfs://%s:%d", config.hostname, config.port);
		off = strlen(url);
		notfirsttime = 1;
	}

	hdfs = hdfsConnect(config.hostname, config.port);
	if (hdfs == NULL) {
		fuse_log(LOG_DEBUG, fname, "Cannot connect to HDFS. Hostname: %s, port: %d", config.hostname, config.port);
		return -EIO;
	}
 
	int count = 0;  
	fileinfo = hdfsListDirectory(hdfs, path, &count);  
	if (fileinfo == NULL) {
		fuse_log(LOG_DEBUG, fname, "Cannot get the file(dir) list in the direcotory: %s", path);
		return -ENOENT;		// the file does NOT exist
	}	

	int i, ret;
	struct stat st; 
	char *root, *dir;
	for (i = 0; i<count; i++) {    
		memset(&st, 0, sizeof(struct stat));  
		st.st_mode 	= (fileinfo->mKind == kObjectKindDirectory)?S_IFDIR:S_IFREG;
		st.st_size		= (fileinfo->mKind == kObjectKindDirectory)?4096:fileinfo->mSize;
		st.st_nlink 	= 1;
		st.st_atime    	= fileinfo->mLastMod;  
		st.st_ctime    	= fileinfo->mLastMod;
		st.st_mtime    	= fileinfo->mLastMod;  
		st.st_blksize  	= 1024;  
		st.st_blocks   	= ceil(st.st_size/st.st_blksize);
		st.st_uid      	= 99;  
		st.st_gid      	= 99; 	 
		
		root = fileinfo[i].mName+off;
		dir = strstr(root, path)+strlen(path);
		if (strchr(dir, '/')) {
			dir++;
		}
		
		if ((ret = filler(buf, dir,&st,0)) != 0) {      
			fuse_log(LOG_DEBUG, fname, "Cannot display the file %s", dir);
		} 
	} 
	
	char *parents[] = { ".",".."}; 
	for (i = 0 ; i < 2 ; i++)    {   
		memset(&st, 0, sizeof(struct stat));  
		st.st_mode 	= (fileinfo->mKind == kObjectKindDirectory)?S_IFDIR:S_IFREG;
		st.st_size		= 4096;
		st.st_nlink 	= 1;
		st.st_atime    	= fileinfo->mLastMod;  
		st.st_ctime    	= fileinfo->mLastMod;
		st.st_mtime    	= fileinfo->mLastMod;  
		st.st_blksize  	= 1024;  
		st.st_blocks   	=  ceil(st.st_size/st.st_blksize);
		st.st_uid      	= 99;  
		st.st_gid      	= 99; 	    
    	
		if ((ret = filler(buf, parents[i],&st,0)) != 0) {     
			fuse_log(LOG_DEBUG, fname, "Cannot display the directory %s", parents[i]);;   
		}  
	}  
	
	hdfsFreeFileInfo(fileinfo, count); 
	return 0;
}

/*     
struct statvfs {    
	unsigned long  f_bsize;    	// file system block size  
	unsigned long  f_frsize;   	// fragment size  
	fsblkcnt_t     f_blocks;   		// size of fs in f_frsize units
	fsblkcnt_t     f_bfree;    		// # free blocks     
	fsblkcnt_t     f_bavail;   		// # free blocks for non-root  
	fsfilcnt_t     f_files;    		// # inodes   
	fsfilcnt_t     f_ffree;    		// # free inodes   
	fsfilcnt_t     f_favail;   		// # free inodes for non-root  
	unsigned long  f_fsid;     	// file system id  
	unsigned long  f_flag;     	// mount flags     
	unsigned long  f_namemax;  // maximum filename length     
};  */ 
static int hdfs_statfs(const char *path, struct statvfs *st)
{ 
	static char fname[] =  "hdfs_statfs";
	hdfsFS hdfs;
	long total, free, used;
	
	if (!path || !st) {
		fuse_log(LOG_DEBUG, fname, "Missing arguments.");
		return -EINVAL;
	}
	if ((hdfs = hdfsConnect(config.hostname, config.port)) == NULL) {
		fuse_log(LOG_DEBUG, fname, "Cannot connect with HDFS. Hostname: %s, port: %d", config.hostname, config.port);
		return -EIO;
	}

	memset(st, 0,sizeof(struct statvfs)); 
	total = hdfsGetCapacity(hdfs); 
	used = hdfsGetUsed(hdfs); 
	free = total-used;
	if (!(total & used)) {
		fuse_log(LOG_DEBUG, fname, "Cannot get the filesystem statistics %s", path);
	}
	
	st->f_bsize   		=  hdfsGetDefaultBlockSize(hdfs); 
	st->f_frsize  		=  st->f_bsize;  
	st->f_blocks  		=  total/st->f_bsize;
	st->f_bfree   		=  free/st->f_bsize;  
	st->f_bavail 		=  st->f_bfree;  
	st->f_files   		=  1000;  
	
	return 0;
}

/*
struct fuse_operations ops = {    
	NULL,				// int(*  getattr )(const char *, struct stat *) 
	NULL,				// int(*  readlink )(const char *, char *, size_t) 
	NULL, 				// int(*  mknod )(const char *, mode_t, dev_t) 
	NULL,				// int(*  mkdir )(const char *, mode_t) 
	hdfs_unlink,			// int(*  unlink )(const char *) 
	NULL,				// int(*  rmdir )(const char *) 
	NULL,				// int(*  symlink )(const char *, const char *) 
	hdfs_rename,		// int(*  rename )(const char *, const char *) 
	NULL, 				// int(*  link )(const char *, const char *) 
	NULL,				// int(*  chmod )(const char *, mode_t) 
	NULL,				// int(*  chown )(const char *, uid_t, gid_t) 
	NULL,				// int(*  truncate )(const char *, off_t) 
	NULL,				// int(*  utime )(const char *, struct utimbuf *) 
	hdfs_open,			// int(*  open )(const char *, struct fuse_file_info *) 
	hdfs_read,			// int(*  read )(const char *, char *, size_t, off_t, struct fuse_file_info *) 
	hdfs_write,			// int(*  write )(const char *, const char *, size_t, off_t, struct fuse_file_info *) 
	NULL,				// int(*  statfs )(const char *, struct statvfs *) 
	NULL,				// int(*  flush )(const char *, struct fuse_file_info *) 
	hdfs_release,		// int(*  release )(const char *, struct fuse_file_info *) 
	NULL,				// int(*  fsync )(const char *, int, struct fuse_file_info *) 
	NULL,				// int(*  setxattr )(const char *, const char *, const char *, size_t, int) 
	NULL,				// int(*  getxattr )(const char *, const char *, char *, size_t) 
	NULL,				// int(*  listxattr )(const char *, char *, size_t) 
	NULL,				// int(*  removexattr )(const char *, const char *) 
	NULL,				// int(*  opendir )(const char *, struct fuse_file_info *) 
	NULL,				// int(*  readdir )(const char *, void *, fuse_fill_dir_t, off_t, struct fuse_file_info *) 
	NULL,				// int(*  releasedir )(const char *, struct fuse_file_info *) 
	NULL,				// int(*  fsyncdir )(const char *, int, struct fuse_file_info *) 
	hdfs_init,			// void *(*  init )(struct fuse_conn_info *conn) 
	hdfs_destroy,		// void(*  destroy )(void *) 
	NULL,				// int(*  access )(const char *, int) 
	hdfs_create,			// int(*  create )(const char *, mode_t, struct fuse_file_info *) 
	NULL,				// int(*  ftruncate )(const char *, off_t, struct fuse_file_info *) 
	NULL,				// int(*  fgetattr )(const char *, struct stat *, struct fuse_file_info *) 
	NULL,				// int(*  lock )(const char *, struct fuse_file_info *, int cmd, struct flock *) 
	NULL,				// int(*  utimens )(const char *, const struct timespec tv[2]) 
	NULL,				// int(*  bmap )(const char *, size_t blocksize, uint64_t *idx) 
};
*/

void hdfs_env_init()
{
	struct stat envstat;
	
	if ((env.cfgdir = getenv("HDFS_FUSE_CONF")))
	{
		if (stat(env.cfgdir, &envstat) < 0) 
		{
			env.cfgdir = "/etc/hdfs-fuse/conf";
		}		
	} else {
		env.cfgdir = "/etc/hdfs-fuse/conf";
	}
	if (stat(env.cfgdir, &envstat) < 0) 
	{
		exit(127);
	}
}

void hdfs_config_init()
{
	/* init cfgv array  */
	config.cfgc = 4;
	config.cfgv = (KeyValue*)calloc(config.cfgc, sizeof(KeyValue));
	config.cfgv[0].key = "LogDir";
	config.cfgv[1].key = "LogLevel";
	config.cfgv[2].key = "Hostname";
	config.cfgv[3].key = "Port";
	
	/*	read from config file   */
	if (fuse_config_init(env.cfgdir, "hdfs-fuse", config.cfgv, config.cfgc))  exit(127);
	
	/*	convert from string to real type   */
	config.logdir 		= (config.cfgv[0].value == NULL) ? NULL : strdup(config.cfgv[0].value);
	config.hostname 	= (config.cfgv[2].value == NULL) ? NULL : strdup(config.cfgv[2].value);
	config.port 		= (config.cfgv[3].value == NULL) ?  0 : atoi(config.cfgv[3].value);
	int i;
	for (i=0; i<8; i++)
	{
		if (strcmp(config.cfgv[1].value, log_level_array[i]) == 0)
		{
			config.loglv = (Log_Level)i;
			break;
		}
	}
	
	/*	free mem  */
	for (i=0; i<config.cfgc; i++)
	{
		free(config.cfgv[i].value);
	}
	free(config.cfgv);
	return;
}

static struct fuse_operations ops;
	
int main(int argc, char *argv[])
{	
	ops.open 		= hdfs_open;
	ops.read 		= hdfs_read;
	ops.create 		= hdfs_create;
	ops.write 		= hdfs_write;
	ops.rename 		= hdfs_rename;
	ops.unlink 		= hdfs_unlink;
	ops.release 		= hdfs_release;
	ops.getattr		= hdfs_getattr;
	ops.readdir		= hdfs_readdir;
	ops.statfs		= hdfs_statfs;
	ops.init 			= hdfs_init;
	ops.destroy 		= hdfs_destroy;

	hdfs_env_init();
	hdfs_config_init();
	//config.logdir = "/tmp";
	//config.loglv = LOG_DEBUG;
	//config.hostname = "localhost";
	//config.port = 9000;

	if (fuse_log_init(config.logdir, "hdfs-fuse", config.loglv) ) exit(127);	
	fuse_log(LOG_DEBUG, "main", "log init done");
	return fuse_main(argc, argv, &ops, NULL);
}	

