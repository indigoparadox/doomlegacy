// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
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
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <direct.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifndef __WIN32__
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <string.h>

#include "../filesrch.h"
#include "../m_misc.h"

//
// filesearch:
//
// ATTENTION : make sure there is enough space in filename to put a full path (255 or 512)
// if needmd5check==0 there is no md5 check
// if changestring then filename will be change with the full path and name
// maxsearchdepth==0 only search given directory, no subdirs
// return FS_NOTFOUND
//        FS_MD5SUMBAD;
//        FS_FOUND

filestatus_e filesearch(char *filename, char *startpath, unsigned char *wantedmd5sum, 
                        boolean changestring, int maxsearchdepth)
{
    struct _finddata_t dta;
    int    searchhandle;
    
    searchhandle=access(filename,4);
    if(searchhandle!=-1)
    {
        if(true)
        {
            // take care of gmt timestamp conversion
            if( checkfilemd5(filename,wantedmd5sum) )
                return FS_FOUND;
            else
                return FS_MD5SUMBAD; // found with differant date
        }
        else
            return FS_FOUND;
    }
    
    if((searchhandle=_findfirst("*.*",&dta))!=-1)
    {
        do {
            if((dta.name[0]!='.') && (dta.attrib & _A_SUBDIR ))
            {
                if( chdir(dta.name)==0 ) { // can fail if we haven't the right
                    int found;
                    found = filesearch(filename,NULL,wantedmd5sum,changestring,10);
                    chdir("..");
                    if( found )
                    {
                        if(changestring)
                            strcatbf(filename,dta.name,"\\");
                        _findclose(searchhandle);
                        return found;
                    }
                }
            }
        } while(_findnext(searchhandle,&dta)==0);
    }
    _findclose(searchhandle);
    return FS_NOTFOUND;
}

