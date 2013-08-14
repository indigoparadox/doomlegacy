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
// $Log: i_system.c,v $
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
// Revision 1.1  2001/04/17 22:23:38  calumr
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
// Initial add
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
//
// Revision 1.1  2000/08/21 21:17:32  metzgermeister
// Initial import to CVS
//
//
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include <Carbon/Carbon.h>

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"
#include "d_net.h"
#include "g_game.h"
#include "endtxt.h"
#include "i_joy.h"

JoyType_t Joystick;


uint16_t scantokey [128] =
{
  '=',  '9',  '7',  '-',  '8',  '0',  ']',  'o',
  'y',	't',  '1',  '2',  '3',  '4',  '6',  '5',
  'c',	'v',    0,  'b',  'q',  'w',  'e',  'r',
  'a',	's',  'd',  'f',  'h',  'g',  'z',  'x',
  KEY_LSHIFT, KEY_CAPSLOCK, KEY_LALT, KEY_LCTRL, 0, 0, 0, 0,
  KEY_TAB, KEY_SPACE, KEY_CONSOLE, KEY_BACKSPACE, 0, KEY_ESCAPE, 0, KEY_RALT,
  'k',	';', '\\',  ',',  '/',  'n',  'm',  '.',
  'u',	'[',  'i',  'p',  KEY_ENTER,  'l',  'j',  '\'',
  KEY_KEYPAD6, KEY_KEYPAD7, 0, KEY_KEYPAD8, KEY_KEYPAD9, 0, 0, 0,
  0, KEY_EQUALS, KEY_KEYPAD0, KEY_KEYPAD1, KEY_KEYPAD2, KEY_KEYPAD3, KEY_KEYPAD4, KEY_KEYPAD5,
  0, 0, 0, KEY_KPADSLASH, KEY_KPADENTER, 0, KEY_MINUSPAD, 0,
  '`',  KEY_KPADPERIOD, 0,  '*',  0, KEY_PLUSPAD, 0, KEY_NUMLOCK,
  KEY_F2, KEY_PGDN, KEY_F1, KEY_LEFTARROW, KEY_RIGHTARROW, KEY_DOWNARROW, KEY_UPARROW, 0,
  0, 0, KEY_INS, KEY_HOME, KEY_PGUP, KEY_DELETE, KEY_F4, KEY_END,
  0, 0, 0, 0, 0, KEY_F10, 0, KEY_F12,
  KEY_F5, KEY_F6, KEY_F7, KEY_F3, KEY_F8, KEY_F9, 0, KEY_F11,
};


int I_GetKey(void)
{
    KeyMap keymap;
    byte newBit;
    int i,n;
	
    GetKeys ((unsigned long *) keymap);
	
    for (i=0;i<4;i++)
    {
        if (keymap[i])
        {
	    for (n=0;n<32;n++)
	    {
	        newBit = ((keymap[i] >> n) & 1);
	        if (newBit)
		    return scantokey[i*32+n];
	    }
	}
    }
    return 0;
}

//
// I_OutputMsg
// Outputs message to log.txt file in Legacy folder.
//
#include <fcntl.h>
void I_OutputMsg (char *error, ...)
{
#ifdef DEBUG_TO_FILE
    int handle;
static int wipe = 1;
    va_list     argptr;
    char        txt[1024];
	
    va_start (argptr,error);
    vsprintf (txt,error,argptr);
    va_end   (argptr);
	
	printf(txt);	// Woohoo! MacOSX command-line output!
	
	if (wipe)
	{
		handle = open ("log.txt", O_WRONLY | O_APPEND | O_BINARY | O_TRUNC | O_CREAT, 0666);
		wipe = 0;
	}
	else
		handle = open ("log.txt", O_RDWR | O_APPEND | O_BINARY, 0666);
	
	if (handle == -1)
		return;
	
    write(handle, txt, strlen(txt));
    close(handle);
#endif
}

//----------------------------------------------
//----------------------------------------------
// cwd (current working directory) does not apply
// to mac apps (unless run from terminal). Make
// cwd = app directory. 

void GetApplicationFSSpec(FSSpec *me)
{
	ProcessInfoRec info;
	ProcessSerialNumber psn = { 0,kCurrentProcess };
	OSErr err;

	GetCurrentProcess(&psn);
	memset((char*)&info,0,sizeof(info));	
	info.processInfoLength = sizeof(info);
	info.processAppSpec = me;
	err = GetProcessInformation(&psn,&info);
}
#include <unistd.h>
#include <fcntl.h>
char *I_GetWadDir(void)
{
	FSSpec app_spec;
	FSSpec app_dir_spec;
	FSRef app_dir;
	UInt32 len = 256;
	char *path;
	OSErr err;
	
	GetApplicationFSSpec(&app_spec);
	
	FSMakeFSSpec(app_spec.vRefNum, app_spec.parID, "", &app_dir_spec);
	FSMakeFSSpec(app_dir_spec.vRefNum, app_dir_spec.parID, "", &app_spec);
	FSMakeFSSpec(app_spec.vRefNum, app_spec.parID, "", &app_dir_spec);
	FSMakeFSSpec(app_dir_spec.vRefNum, app_dir_spec.parID, "", &app_spec);
		
	err = FSpMakeFSRef(&app_spec, &app_dir);
	
	path = malloc(256);
	err = FSRefMakePath(&app_dir, path, len);
	
	I_OutputMsg("You double-clicked me!\n");
	
	chdir(path);
	
	return path;
}

ULONG I_GetFreeMem(ULONG *total)
{
  // add 0x01 to indicate guess
	*total = MaxBlock() - 4*1024*1024;
	return 0;  // no freemem
}

static int quiting=0; /* prevent recursive I_Quit() */

void I_Tactile(int on,int off,int total )
{
  // Used for force feedback. 
  on = off = total = 0;
}

ticcmd_t        emptycmd;
ticcmd_t* I_BaseTiccmd(void)
{
    return &emptycmd;
}


int  I_GetHeapSize (void)
{
    return MaxBlock() - 4*1024*1024;	//Fix this too. 
}

byte* I_ZoneBase (int*  size)
{
    return NULL;
}

//
// I_GetTime
// returns time in 1/TICRATE second tics
//

void I_StartupTimer (void)
{}

ULONG  I_GetTime (void)
{
    UnsignedWide ftime;
    static double baseTicks=0;
    static UInt32 hiTicks=0;
    double ticks;
	
	Microseconds(&ftime);
	
	ticks = ftime.lo/1000000.0f;
	
	if (!baseTicks)
	{
		baseTicks = ticks;
		hiTicks = ftime.hi;
	}
	
	if (hiTicks != ftime.hi)
	{
	    baseTicks = 0;
	}
	
	hiTicks = ftime.hi;
	
    return (ticks - baseTicks) * TICRATE;
}

//
// I_Init
//
void I_Init (void)
{
    MenuBarHandle menuBar;
    
	/*menuBar = GetNewMBar(rMenuBar);
    SetMenuBar(menuBar);	
	DisposeHandle(menuBar);*/
    
    I_StartupSound();
    I_InitMusic();
    quiting = 0;
}

//
// I_Quit
//
void I_Quit (void)
{
    /* prevent recursive I_Quit() */
    if(quiting)
        return;
    quiting = 1;
    if (demorecording)
        G_CheckDemoStatus();
    W_Shutdown();
    D_QuitNetGame ();
    I_ShutdownMusic();
    I_ShutdownSound();
#ifdef CDMUS
    I_ShutdownCD();
#endif
    M_SaveConfig (NULL);
    I_ShutdownGraphics();
    I_ShutdownInput();
    I_OutputMsg("Nice knowing you...\n");
    ExitToShell();
}

void I_WaitVBL(int count)
{}

void I_BeginRead(void)
{
      //can be used to show a "loading..." thing
      //but it would appear for only a fraction of a second.
}

void I_EndRead(void)
{}

//
// I_Error
//
extern boolean demorecording;

void I_Error (char *error, ...)
{
    va_list     argptr;
    char        txt[1024];

    va_start (argptr,error);
    vsprintf (txt,error,argptr);
    va_end   (argptr);
    
    {
		SInt16 res;
		c2pstr(txt);
		
        StandardAlert(kAlertStopAlert,"\pError:",(ConstStr255Param)txt,NULL,&res);
    }

    // Shutdown. Here might be other errors.
    if (demorecording)
        G_CheckDemoStatus();
	W_Shutdown();
    D_QuitNetGame ();
    I_ShutdownMusic();
    I_ShutdownSound();
    I_ShutdownGraphics();
    I_ShutdownInput();
    
    I_OutputMsg("%s\nFeck. I didn't see that one coming. \n",txt);
    ExitToShell();
}

void I_GetDiskFreeSpace(long long *freespace) {
    // 10MB should be enough
    *freespace = 10*1024*1024;
}

char *I_GetUserName(void)
{
	return getenv("USER");
}

int  I_mkdir(const char *dirname, int unixright)
{
    return mkdir(dirname, unixright);
}

void I_LocateWad(void)
{
}
