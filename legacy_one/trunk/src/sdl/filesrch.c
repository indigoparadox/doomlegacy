// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Portions Copyright (C) 1998-2015 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//  Search directories, in depth, for a filename.
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "filesrch.h"
//#include "d_netfil.h"

//
// filesearch:
//
// ATTENTION : make sure there is enough space in filename to put a full path (255 or 512)
//    filename must be buffer of MAX_WADPATH
// if needmd5check==0 there is no md5 check
// if completepath then filename will be changed to the full path and name
// maxsearchdepth==0 only search given directory, no subdirs
// return FS_NOTFOUND
//        FS_MD5SUMBAD;
//        FS_FOUND
#define MAX_SRCHPATH (MAX_WADPATH * 2)

filestatus_e filesearch(char *filename, char *startpath, unsigned char *wantedmd5sum, boolean completepath, int maxsearchdepth)
{
    DIR ** dirhandle_stack;  // (malloc)
    DIR * dirhandle;
    struct dirent *dent;
    struct stat fstat;
    char * searchname = strdup( filename);  // (malloc)
    filestatus_e retval = FS_NOTFOUND;
    int remspace;
    int depthleft=maxsearchdepth;
    int * index_stack;  // each directory in the searchpath  (malloc)
    int cur_index;
    char searchpath[MAX_SRCHPATH];

    dirhandle_stack = (DIR**) malloc( maxsearchdepth * sizeof( DIR*));
    if( dirhandle_stack == NULL )   goto error1_exit;
    index_stack = (int*) malloc( maxsearchdepth * sizeof( int));
    if( index_stack == NULL )   goto error2_exit;
    
    strncpy( searchpath, startpath, MAX_SRCHPATH-1 );
    searchpath[MAX_SRCHPATH-1] = '\0';
    cur_index = strlen( searchpath) + 1;

    dirhandle = opendir( searchpath);
    if( dirhandle == NULL )  goto error3_exit;

    // Initial stack
    index_stack[--depthleft] = cur_index;
    dirhandle_stack[ depthleft ] = dirhandle;

    if(searchpath[cur_index-2] != '/')
    {
        searchpath[cur_index-1] = '/';
        searchpath[cur_index] = 0;
    }
    else
    {
        cur_index--;
    }

    while( depthleft < maxsearchdepth )
    {
        searchpath[cur_index]=0;
        dent = readdir( dirhandle );  // next dir entry
        if( !dent)  // done with dir
        {
            closedir( dirhandle );
            // Pop stack to previous directory.
            depthleft++;
            cur_index = index_stack[depthleft];
            dirhandle = dirhandle_stack[depthleft];
            continue;
        } 
        if( dent->d_name[0]=='.' )
        {
            // ignore the "." and ".." entries, we don't want to scan uptree
            if( dent->d_name[1]=='\0' )  continue;
            if( dent->d_name[1]=='.' && dent->d_name[2]=='\0' )  continue;
        }

        // append dir name
        remspace = (MAX_SRCHPATH - 1) - cur_index;
        strncpy(&searchpath[cur_index], dent->d_name, remspace);

        if( stat(searchpath,&fstat) < 0) // do we want to follow symlinks? if not: change it to lstat
        {
            // was the file (re)moved? can't stat it
	    continue;
        }

        if( S_ISDIR(fstat.st_mode) )
        {
	    if( depthleft <= 0 )  continue;  // depth limited
            remspace = (MAX_SRCHPATH - 1) - cur_index;
            strncpy(&searchpath[cur_index], dent->d_name, remspace);

            dirhandle = opendir(searchpath);
            if( dirhandle == NULL )
            {
                // can't open it... maybe no read-permissions
                // go back to previous dir
                cur_index = index_stack[depthleft];
                dirhandle = dirhandle_stack[depthleft];
                continue;
            }

            // Push stack to new directory.
            cur_index = strlen(searchpath) + 1;
            index_stack[--depthleft] = cur_index;
            dirhandle_stack[depthleft] = dirhandle;

            searchpath[cur_index-1]='/';
            searchpath[cur_index]=0;
        }
        else if (!strcasecmp(searchname, dent->d_name))
        {
            switch( checkfilemd5(searchpath, wantedmd5sum))
            {
                case FS_FOUND:
                    if(completepath)
                    {
                        strncpy(filename, searchpath, MAX_WADPATH-1);
                        filename[MAX_WADPATH-1] = '\0';
                    }
#if 0
// [WDJ] This is used for "find if file exists",
// which is not choice to return the dir name instead.
// If this is ever needed, it requires a separate enable flag.
                    else
                    {
                        strncpy(filename, dent->d_name, MAX_WADPATH-1);
                        filename[MAX_WADPATH-1] = '\0';
                    }
#endif
                    retval=FS_FOUND;
                    goto found_exit;
                case FS_MD5SUMBAD:
                    retval = FS_MD5SUMBAD;
                    break;
                default:
                    // prevent some compiler warnings
                    break;
            }
        }
    }

found_exit:
    for(; depthleft<maxsearchdepth; closedir(dirhandle_stack[depthleft++]));

error3_exit:
    free(index_stack);
error2_exit:
    free(dirhandle_stack);
error1_exit:
    free(searchname);

    return retval;
}
