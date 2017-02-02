/*
 * Copyright (c) 2016 Red Hat.
 * Copyright (c) 2010 Aconex.  All Rights Reserved.
 * Copyright (c) 2000,2004 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>
#include "pmapi.h"

/*
 * Convert kernels string device number encoding into a dev_t.
*/
dev_t
get_encoded_dev(const char *devnum)
{
    unsigned device = (unsigned int)strtoul(devnum, NULL, 0);
    return (dev_t)device;
}

/*
 * Attempt to map a device number to a tty for a given process.
 *
 * Previously this was much more elaborate, scanning all open fds
 * for a match on the device; but that is expensive for processes
 * with many open fds, and we end up stat'ing all sorts of files
 * unrelated to the job at hand (which SElinux blocks and reports
 * as poor form).
 *
 * Returns a pointer into a static buffer, so no free'ing needed.
 */
char *
get_ttyname_info(int pid, dev_t dev)
{
    struct stat statbuf;
    char fullpath[MAXPATHLEN];
    static char ttyname[MAXPATHLEN];
    char *devpath = "/dev/";
    struct dirent *drp;
    DIR *rundir;

    strcpy(ttyname, "?");
    if ((rundir = opendir(devpath)) == NULL){
	return ttyname;
    }
    while ((drp = readdir(rundir)) != NULL) {
	char *path;
	if (*(path = &drp->d_name[0]) == '.')
	    continue;
	sprintf(fullpath, "%s%s", devpath, path);
	if(!stat(fullpath, &statbuf) &&
	   S_ISCHR(statbuf.st_mode)) {
	    if (dev == statbuf.st_rdev) {
		strcpy(ttyname, &fullpath[5]);
		break;
	    }
	}
    }
    return ttyname;
}
