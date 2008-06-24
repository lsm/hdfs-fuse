/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * log.c
 * Copyright (C) Peng Zhao 2008 <jass.zhao@gmail.com>
 * 
 * log.c is free software.
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
 
#include <hdfs-fuse.h>

#include <sys/stat.h>
#include <time.h>
#include <pthread.h>

static char logpath[MAX_FILENAME_LENGTH];
static pthread_mutex_t mutex_log;
static Log_Level log_level;

int fuse_log_file(char *logfile, const char *exename, const char *hostname) 
{
	FILE *fp;
	struct stat stat;
    	int create = 0; 
     	 
    if (lstat(logfile, &stat) < 0)	create = 1;
    
    if ((fp = fopen(logfile, "a+")) == NULL) 	// Cannot create the log file
	{	/* fail to open the log file, try tmp */
       	sprintf(logfile, "/tmp/%s.%s.log", exename, hostname);
		if (lstat(logfile, &stat) < 0) 
		{
	   		if (errno == ENOENT) 
			{ 	/* file not exist */
	       		if ((fp = fopen(logfile, "a+")) == NULL) 
				{
	    				return -1;		// Cannot create the log file
				} else {
	    				create = 1;		// fopen() creates the log file automatically
				}
	    		} else {
				return -1;
	    		} 
		} else if (S_ISREG(stat.st_mode) && stat.st_nlink == 1) 
		{	/* file is a regular one, not symlink, neither hard link */
	    		if ((fp = fopen(logfile, "a+")) == NULL)   return -1;
		} else { 
			/* not a regular file */
	    		return -1;
		}
    	}

    	if (fp != NULL) 
	{
        	fclose(fp);
            	chmod(logfile, 0644);
        	return 0;
    	}
    	return 0;
}

int fuse_log_init(const char *path, const char *exename, Log_Level level)
{
	int ret;
	
    	if (path && *path && strlen(path) > MAX_FILENAME_LENGTH)
    	{
		return -1;
	}

	if (pthread_mutex_init(&mutex_log, NULL))
	{
		return -1;
	}
	
	log_level = level;

	char hostname[MAX_FILENAME_LENGTH];
	FILE *fp;
	struct stat stat;

	gethostname(hostname, MAX_HOSTNAME_LENGTH);
	sprintf(logpath, "%s/%s.%s.log", path, exename, hostname);		// rpointd.hostname.log

	if (lstat(logpath, &stat) < 0) 
	{ 
		if (errno == ENOENT) 
		{ 	 /* file not exist */ 
			 ret = fuse_log_file(logpath, exename, hostname);
			 if (!ret) 
			 {
				return 0;
			 } else { /* try to open log file under "tmp" */
				sprintf(logpath, "/tmp/%s.%s.log", exename, hostname);
				if (lstat(logpath, &stat) < 0) 
				{
					if (errno == ENOENT) 
					{ 	/* file not exist */
             					if ((fp = fopen(logpath, "a+")) != NULL) 	// Append mode
						{ 	    
		    					fclose(fp);	       
							chmod(logpath, 0644);
		    					return 0; 
             					}
	    				}
      				} else if (S_ISREG(stat.st_mode) && stat.st_nlink == 1) 
      				{
  		                    /* file is a regular one, not symlink, neither hard link */
           				if ((fp = fopen(logpath, "a+")) != NULL) 
					{
			              	fclose(fp);
                   				chmod(logpath, 0644);
                    				return 0;		// Error
             				}
					return -1;			// Success
          			}
        		}
		}
	} else if (S_ISREG(stat.st_mode) && stat.st_nlink == 1) 
	{	/* file is a regular one, not symlink, neither hard link */
		 return fuse_log_file(logpath, exename, hostname);
	} 
	return 0;	// we should NOT be here
}

/*
Time:PID:TID:Level:fname:"%d %s", arg....
2008-06-08(2:06:32):17124:1:LOG_INFO:func():xxxxxxx
*/
void fuse_log_msg(FILE *fp, Log_Level level, const char *fname, const char *msg, va_list ap)
{
	static char lastmsg[16384], newmsg[16384];
	static pid_t lastpid;
	static pthread_t lasttid, nowtid;
    	static int  count;
    	static time_t lastime, lastcall;
    	time_t now; 

    	vsprintf(newmsg, msg, ap);		
       time(&now); 
	nowtid = (pthread_self()<0)?1:(int)pthread_self();
	
    	if (lastmsg[0] && (strcmp(newmsg, lastmsg) == 0) && (now - lastime < 600) &&
	     lastpid == (pid_t)getpid() && lasttid == nowtid)
	{
       	count++;
		lastcall = now;
        	return;
    	} else {
       	/* time stamp */
		char *ctime, *cend;
		ctime = asctime(localtime(&now));
		cend = ctime+strlen(ctime)-1;
		*cend = '\0';
        	if (count) 
		{
	    		(void)fprintf(fp, "%s:%d:%ld:", ctime, lastpid, lasttid);
            		(void)fprintf(fp, "Last message repeated %d times\n", count);   
        	}
        	(void)fprintf(fp, "%s:%d:%ld:%s:%s():", ctime, (int) getpid(), nowtid, log_level_array[(int)level], fname);
    	}

    	fputs(newmsg, fp);
    	putc('\n', fp);
    	fflush(fp);
		
    	/* Save the last message */
    	strcpy(lastmsg, newmsg);
   	count = 0;
    	lastime = now;
	lastpid = (pid_t)getpid();
	lasttid = nowtid;
	
	return;
} 

void fuse_log (Log_Level level, const char *fname, const char *msg, ...)
{
	int errno;
    	va_list ap;

    	va_start(ap, msg);

	if (level >= log_level) 
	{
		FILE *fp;
	    	struct stat stat;

            	pthread_mutex_lock(&mutex_log);

	    	if (lstat(logpath, &stat) < 0) 
		{
			if (errno == ENOENT) 
			{ 	/* file not exist */
	    	    		if ((fp = fopen(logpath, "a+")) == NULL) 
				{
        	        		pthread_mutex_unlock(&mutex_log);
		        		return;
	    	    		}
			} else { 
				/* file exists, but fails to stat */
        	        	pthread_mutex_unlock(&mutex_log);
		        	return;
			}
		} else if (!(S_ISREG(stat.st_mode) && stat.st_nlink == 1)) 
	    	{
           		/* not a regular file */ 
			if ((fp = fopen(logpath, "a+")) == NULL) 
			{
				pthread_mutex_unlock(&mutex_log);	
		        	return;
			}
	    	} else { 
	    		/* regular file */	        
			if ((fp = fopen(logpath, "a+")) == NULL) 
			{
        	        	pthread_mutex_unlock(&mutex_log);	
		        	return;
			}
	    	}

	    	fuse_log_msg(fp, level, fname, msg, ap);
	    	fclose(fp);
	    	pthread_mutex_unlock(&mutex_log);
	}
    	va_end(ap);
	return;
}
