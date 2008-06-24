/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * cfg.c
 * Copyright (C) Peng Zhao 2008 <jass.zhao@gmail.com>
 * 
 * cfg.c is free software.
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
#include <ctype.h>

void fuse_config_read(FILE *fp, KeyValue cfgv[], int cfgc)
{
    	char line[1025], *value, *c;
    	int num_line = 0;
	KeyValue *temp;
        
	while (fgets(line, 1024, fp) != NULL)
    	{	/*	read a line & trim   */
       	num_line++;
       	if (line[0] == '\0' || line[0] == '#' || line[0] == '\n') continue;
    		c = line + strlen(line) - 1;
    		while (isspace(*c) && c > line) {
        		*c = '\0';
        		c--;
    		}
       	if (line[0] == '\0')  continue;
        	value = line;
        	while (!isspace(*value))  value++;
        	if (*value== '\0') {
           		value = NULL;
        	} else {
            		*value = '\0';
            		value++;
        	}
		/*	find the key   */	
    		for (temp = cfgv; temp < cfgv + cfgc; temp++)
    		{
        		if (strcasecmp(line, temp->key) == 0)  break;
    		}
		/*	set the config value   */
    		if (temp->value) {
            		if (temp->value!= NULL)  free(temp->value);
            		temp->value = strdup(value);
        	}
    	}
	return;
}


int fuse_config_init(const char * cfgdir,const char *exe, KeyValue *cfgv, int cfgc)
{
	char cfile[MAX_FILENAME_LENGTH];
    	char line[1025], *value, *c;
	FILE *fp;
    	int num_line = 0;
	KeyValue *temp;

	memset((void*)cfile, 0, MAX_FILENAME_LENGTH);
	memset((void*)line,  0, 1025);
	
	sprintf(cfile, "%s/%s.conf", cfgdir, exe);
	if ((fp = fopen(cfile, "r")) == NULL)
	{
		return -1;
	}
	
	while (fgets(line, 1024, fp) != NULL)
    	{	/*	read a line & trim   */
       	num_line++;
       	if (line[0] == '\0' || line[0] == '#' || line[0] == '\n') continue;
    		c = line + strlen(line) - 1;
    		while (isspace(*c) && c > line) {
        		*c = '\0';
        		c--;
    		}
       	if (line[0] == '\0')  continue;
        	value = line;
        	while (!isspace(*value))  value++;
        	if (*value== '\0') {
           		value = NULL;
        	} else {
            		*value = '\0';
            		value++;
        	}
		/*	find the key   */	
    		for (temp = cfgv; temp < cfgv + cfgc; temp++)
    		{
        		if (strcasecmp(line, temp->key) == 0) 
        		{	/*	set the config value   */
            			temp->value = strdup(value);
				memset((void*)line,  0, 1025);
				break;
			}
    		}
    	}

	fclose(fp);

	return 0;
}


