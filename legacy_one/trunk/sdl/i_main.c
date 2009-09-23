// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
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
//
// $Log: i_main.c,v $
// Revision 1.4  2003/06/05 20:36:10  hurdler
// do not write log.txt if no .log directory exists
//
// Revision 1.3  2002/09/10 19:30:27  hurdler
// Add log file under Linux
//
// Revision 1.2  2000/09/10 10:56:00  metzgermeister
// clean up & made it work again
//
// Revision 1.1  2000/08/21 21:17:32  metzgermeister
// Initial import to CVS
//
// 
//
// DESCRIPTION:
//      Main program, simply calls D_DoomMain high level loop.
//
//-----------------------------------------------------------------------------


#include "doomdef.h"

#include "m_argv.h"
#include "d_main.h"

#ifdef LOGMESSAGES
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
int logstream;
#endif

int main(int argc, char **argv)
{ 
    myargc = argc; 
    myargv = argv; 
 
#ifdef LOGMESSAGES
    //Hurdler: only write log if we have the permission in the current directory
    logstream = creat(".log/log.txt", S_IRUSR | S_IWUSR);
    if (logstream < 0)
    {
        logstream = INVALID_HANDLE_VALUE; // so we haven't to change the current source code
    }
#endif

    D_DoomMain ();
    D_DoomLoop ();
    return 0;
} 
