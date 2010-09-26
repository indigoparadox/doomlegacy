// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998-2010 by DooM Legacy Team.
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
// Revision 1.12  2003/05/04 04:24:08  sburke
// Add Solaris support.
//
// Revision 1.11  2002/01/03 19:20:07  bock
// Add FreeBSD code to I_GetFreeMem.
// Modified Files:
//     makefile linux_x/i_system.c sdl/i_system.c
//
// Revision 1.10  2001/12/31 16:56:39  metzgermeister
// see Dec 31 log
// .
//
// Revision 1.9  2001/08/20 20:40:42  metzgermeister
// *** empty log message ***
//
// Revision 1.8  2001/05/16 22:33:35  bock
// Initial FreeBSD support.
//
// Revision 1.7  2001/03/12 21:03:10  metzgermeister
//   * new symbols for rendererlib added in SDL
//   * console printout fixed for Linux&SDL
//   * Crash fixed in Linux SW renderer initialization
//
// Revision 1.6  2001/02/24 13:35:23  bpereira
// no message
//
// Revision 1.5  2000/11/02 19:49:40  bpereira
// no message
//
// Revision 1.4  2000/10/16 21:20:53  hurdler
// remove unecessary code
//
// Revision 1.3  2000/09/26 17:58:06  metzgermeister
// I_Getkey implemented
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
//
//-----------------------------------------------------------------------------


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "SDL.h"


#ifdef LINUX
# ifndef FREEBSD
#  include <sys/vfs.h>
# else
#  include <sys/param.h>
#  include <sys/mount.h>
/*For meminfo*/
#  include <sys/types.h>
#  include <kvm.h>
#  include <nlist.h>
#  include <sys/vmmeter.h>
#  include <fcntl.h>
# endif
#endif

#ifdef LMOUSE2
#include <termios.h>
#endif

#include "doomdef.h"
#include "m_misc.h"
#include "i_video.h"
#include "i_sound.h"
#include "i_system.h"

#include "d_net.h"
#include "g_game.h"

#include "keys.h"
#include "i_joy.h"

#include "endtxt.h"


extern void D_PostEvent(event_t*);

#define MAX_JOYSTICKS 4 // 4 should be enough for most purposes
int num_joysticks = 0;
SDL_Joystick *joysticks[MAX_JOYSTICKS]; 

#ifdef LMOUSE2
int fdmouse2 = -1;
int mouse2_started = 0;
#endif

//
//I_OutputMsg
//
void I_OutputMsg       (char *fmt, ...) 
{
    va_list     argptr;

    va_start (argptr,fmt);
    vfprintf (stderr,fmt,argptr);
    va_end (argptr);

}

//
// I_GetKey
//
int  I_GetKey          (void)
{
    // Warning: I_GetKey emties the event queue till next keypress
    event_t*    ev;
    int rc=0;

    // return the first keypress from the event queue
    while (eventtail != eventhead)
    {
        ev = &events[eventtail];
        if(ev->type == ev_keydown)
        {
            rc = ev->data1;
        }

	eventtail++;
	eventtail = eventtail & (MAXEVENTS-1);
    }

    return rc;
}


//
//  Translates the SDL key into Doom key
//
static int xlatekey(SDLKey sym)
{
  // leave ASCII codes unchanged, as well as most other SDL keys
  if (sym >= SDLK_BACKSPACE && sym <= SDLK_MENU)
    return sym;

  return KEY_NULL;
}


//! Translates a SDL joystick button to a doom key_input_e number.
static int TranslateJoybutton(Uint8 which, Uint8 button)
{
  if (which >= MAXJOYSTICKS) 
    which = MAXJOYSTICKS-1;

  if (button >= JOYBUTTONS)
    button = JOYBUTTONS-1;

  return KEY_JOY0BUT0 + JOYBUTTONS*which + button;
}

int I_JoystickNumAxes(int joynum)
{
  if (joynum < num_joysticks)
    return SDL_JoystickNumAxes(joysticks[joynum]);
  else
    return 0;
}

int I_JoystickGetAxis(int joynum, int axisnum)
{
  if (joynum < num_joysticks)
    return SDL_JoystickGetAxis(joysticks[joynum], axisnum);
  else
    return 0;
}

static int lastmousex = 0;
static int lastmousey = 0;

#ifdef LMOUSE2
extern void I_GetMouse2Event();
#endif

// current modifier key status
boolean shiftdown = false;
boolean altdown = false;


void I_GetEvent()
{
  SDL_Event inputEvent;
  SDLKey sym;
  SDLMod mod;

  event_t event;

#ifdef LMOUSE2
  I_GetMouse2Event();
#endif

  while (SDL_PollEvent(&inputEvent))
    {
      switch (inputEvent.type)
        {
        case SDL_KEYDOWN:
	  event.type = ev_keydown;
	  sym = inputEvent.key.keysym.sym;
	  event.data1 = xlatekey(sym); // key symbol

	  mod = inputEvent.key.keysym.mod; // modifier key states
	  // this might actually belong in D_PostEvent
	  shiftdown = mod & KMOD_SHIFT;
	  altdown = mod & KMOD_ALT;

	  // Corresponding ASCII char, if applicable (for console etc.)
	  // NOTE that SDL handles international keyboards and shift maps for us!
	  Uint16 unicode = inputEvent.key.keysym.unicode; // SDL uses UCS-2 encoding (or maybe UTF-16?)
	  if ((unicode & 0xff80) == 0)
	    {
	      event.data2 = unicode & 0x7F;
	    }
	  else
	    event.data2 = 0; // non-ASCII char

	  D_PostEvent(&event);
	  break;

        case SDL_KEYUP:
	  event.type = ev_keyup;
	  sym = inputEvent.key.keysym.sym;
	  event.data1 = xlatekey(sym);

	  mod = inputEvent.key.keysym.mod; // modifier key states
	  shiftdown = mod & KMOD_SHIFT;
	  altdown = mod & KMOD_ALT;

	  D_PostEvent(&event);
	  break;

        case SDL_MOUSEMOTION:
            if(cv_usemouse.value)
            {
                // If the event is from warping the pointer back to middle
                // of the screen then ignore it.
                if ((inputEvent.motion.x == vid.width/2) &&
                    (inputEvent.motion.y == vid.height/2))
                {
                    lastmousex = inputEvent.motion.x;
                    lastmousey = inputEvent.motion.y;
                    break;
                }
                else
                {
                    event.data2 = (inputEvent.motion.x - lastmousex) << 2;
                    lastmousex = inputEvent.motion.x;
                    event.data3 = (lastmousey - inputEvent.motion.y) << 2;
                    lastmousey = inputEvent.motion.y;
                }
                event.type = ev_mouse;
                event.data1 = 0;

                D_PostEvent(&event);

                // Warp the pointer back to the middle of the window
                //  or we cannot move any further if it's at a border.
                if ((inputEvent.motion.x < (vid.width/2)-(vid.width/4)) ||
                    (inputEvent.motion.y < (vid.height/2)-(vid.height/4)) ||
                    (inputEvent.motion.x > (vid.width/2)+(vid.width/4)) ||
                    (inputEvent.motion.y > (vid.height/2)+(vid.height/4)))
                {
                    SDL_WarpMouse(vid.width/2, vid.height/2);
                }
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
	  if(cv_usemouse.value)
            {
	      if (inputEvent.type == SDL_MOUSEBUTTONDOWN)
                event.type = ev_keydown;
	      else
                event.type = ev_keyup;

	      event.data1 = KEY_MOUSE1 + inputEvent.button.button - SDL_BUTTON_LEFT;
	      event.data2 = 0; // does not correspond to any character
	      D_PostEvent(&event);
            }
	  break;

	case SDL_JOYBUTTONDOWN: 
	  event.type = ev_keydown;
	  event.data1 = TranslateJoybutton(inputEvent.jbutton.which, 
					   inputEvent.jbutton.button);
	  D_PostEvent(&event);
	  break;

	case SDL_JOYBUTTONUP: 
	  event.type = ev_keyup;
	  event.data1 = TranslateJoybutton(inputEvent.jbutton.which, 
					   inputEvent.jbutton.button);
	  D_PostEvent(&event);
	  break;

        case SDL_QUIT:
	  I_Quit();
	  //M_QuitResponse('y');
	  break;

        default:
	  break;
        }
    }
}

#ifdef HAS_SDL_BEEN_FIXED
static void doGrabMouse()
{
  if(SDL_GRAB_OFF == SDL_WM_GrabInput(SDL_GRAB_QUERY))
  {
    SDL_WM_GrabInput(SDL_GRAB_ON);
  }
}
#endif

void doUngrabMouse()
{
  if(SDL_GRAB_ON == SDL_WM_GrabInput(SDL_GRAB_QUERY))
  {
    SDL_WM_GrabInput(SDL_GRAB_OFF);
  }
}

void I_StartupMouse(void)
{
    SDL_Event inputEvent;

    // warp to center
    SDL_WarpMouse(vid.width/2, vid.height/2);
    lastmousex = vid.width/2;
    lastmousey = vid.height/2;
    // remove the mouse event by reading the queue
    SDL_PollEvent(&inputEvent);

#ifdef HAS_SDL_BEEN_FIXED // FIXME
  if(cv_usemouse.value)
    {
      doGrabMouse();
    }
  else
    {
      doUngrabMouse();
    }
#endif
    return;
}


/// Initialize joysticks and print information.
void I_JoystickInit()
{
  // Joystick subsystem was initialized at the same time as video,
  // because otherwise it won't work. (don't know why, though ...)

  num_joysticks = min(MAX_JOYSTICKS, SDL_NumJoysticks());
  CONS_Printf(" %d joystick(s) found.\n", num_joysticks);

  // Start receiving joystick events.
  SDL_JoystickEventState(SDL_ENABLE);

  int i;
  for (i=0; i < num_joysticks; i++)
    {
      SDL_Joystick *joy = SDL_JoystickOpen(i);
      joysticks[i] = joy;
      if (devparm)
	{
	  CONS_Printf(" Properties of joystick %d:\n", i);
	  CONS_Printf("    %s.\n", SDL_JoystickName(i));
	  CONS_Printf("    %d axes.\n", SDL_JoystickNumAxes(joy));
	  CONS_Printf("    %d buttons.\n", SDL_JoystickNumButtons(joy));
	  CONS_Printf("    %d hats.\n", SDL_JoystickNumHats(joy));
	  CONS_Printf("    %d trackballs.\n", SDL_JoystickNumBalls(joy));
	}
    }
}


/// Close all joysticks.
void I_ShutdownJoystick()
{
  CONS_Printf("Shutting down joysticks.\n");
  int i;
  for(i=0; i < num_joysticks; i++)
  {
    CONS_Printf("Closing joystick %s.\n", SDL_JoystickName(i));
    SDL_JoystickClose(joysticks[i]);
    joysticks[i] = NULL;
  }
  
  CONS_Printf("Joystick subsystem closed cleanly.\n");
}


/// initialize SDL
void I_SysInit()
{
  CONS_Printf("Initializing SDL...\n");

  // Initialize Audio as well, otherwise DirectX can not use audio
  if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
      CONS_Printf(" Couldn't initialize SDL: %s\n", SDL_GetError());
      I_Quit();
    }

  // Window title
  SDL_WM_SetCaption(VERSION_BANNER, "Doom Legacy");

  // Enable unicode key conversion
  SDL_EnableUNICODE(1);

  // Initialize the joystick subsystem.
  I_JoystickInit();
}


//
// I_OsPolling
//
void I_OsPolling()
{
  if (!graphics_started)
    return;

  I_GetEvent();
}



#ifdef LMOUSE2
//
// I_GetMouse2Event
//
void I_GetMouse2Event() {
  static unsigned char mdata[5];
  static int i = 0,om2b = 0;
  int di,j,mlp,button;
  event_t event;
  const int mswap[8] = {0,4,1,5,2,6,3,7};
  if(!mouse2_started) return;
  for(mlp=0;mlp<20;mlp++) {
    for(;i<5;i++) {
      di = read(fdmouse2,mdata+i,1);
      if(di==-1) return;
    }
    if((mdata[0]&0xf8)!=0x80) {
      for(j=1;j<5;j++) {
        if((mdata[j]&0xf8)==0x80) {
          for(i=0;i<5-j;i++) { // shift
            mdata[i] = mdata[i+j];
          }
        }
      }
      if(i<5) continue;
    } else {
      button = mswap[~mdata[0]&0x07];
      for(j=0;j<MOUSEBUTTONS;j++) {
        if(om2b&(1<<j)) {
          if(!(button&(1<<j))) { //keyup
            event.type = ev_keyup;
            event.data1 = KEY_2MOUSE1+j;
            D_PostEvent(&event);
            om2b ^= 1 << j;
          }
        } else {
          if(button&(1<<j)) {
            event.type = ev_keydown;
            event.data1 = KEY_2MOUSE1+j;
            D_PostEvent(&event);
            om2b ^= 1 << j;
          }
        }
      }
      event.data2 = ((signed char)mdata[1])+((signed char)mdata[3]);
      event.data3 = ((signed char)mdata[2])+((signed char)mdata[4]);
      if(event.data2&&event.data3) {
        event.type = ev_mouse2;
        event.data1 = 0;
        D_PostEvent(&event);
      }
    }
    i = 0;
  }
}

//
// I_ShutdownMouse2
//
void I_ShutdownMouse2() {
  if(fdmouse2!=-1) close(fdmouse2);
  mouse2_started = 0;
}

#endif

//
// I_StartupMouse2
// 
void I_StartupMouse2 (void) {
#ifdef LMOUSE2
  struct termios m2tio;
  int i,dtr,rts;
  I_ShutdownMouse2();
  if(cv_usemouse2.value == 0) return;
  if((fdmouse2 = open(cv_mouse2port.string,O_RDONLY|O_NONBLOCK|O_NOCTTY))==-1) {
    CONS_Printf("Error opening %s!\n",cv_mouse2port.string);
    return;
  }
  tcflush(fdmouse2, TCIOFLUSH);
  m2tio.c_iflag = IGNBRK;
  m2tio.c_oflag = 0;
  m2tio.c_cflag = CREAD|CLOCAL|HUPCL|CS8|CSTOPB|B1200;
  m2tio.c_lflag = 0;
  m2tio.c_cc[VTIME] = 0;
  m2tio.c_cc[VMIN] = 1;
  tcsetattr(fdmouse2, TCSANOW, &m2tio);
  strupr(cv_mouse2opt.string);
  for(i=0,rts = dtr = -1;i<strlen(cv_mouse2opt.string);i++) {
    if(cv_mouse2opt.string[i]=='D') {
      if(cv_mouse2opt.string[i+1]=='-') {
        dtr = 0;
      } else {
        dtr = 1;
      }
    }
    if(cv_mouse2opt.string[i]=='R') {
      if(cv_mouse2opt.string[i+1]=='-') {
        rts = 0;
      } else {
        rts = 1;
      }
    }
  }
  if((dtr!=-1)||(rts!=-1)) {
    if(!ioctl(fdmouse2, TIOCMGET, &i)) {
      if(!dtr) {
        i &= ~TIOCM_DTR;
      } else {
        if(dtr>0) i |= TIOCM_DTR;
      }
      if(!rts) {
        i &= ~TIOCM_RTS;
      } else {
        if(rts>0) i |= TIOCM_RTS;
      }
      ioctl(fdmouse2, TIOCMSET, &i);
    }
  }
  mouse2_started = 1;
#endif
}

byte     mb_used = 6+2; // 2 more for caching sound

//
// I_Tactile
//
void I_Tactile(int on,int off,int total )
{
  // UNUSED.
  on = off = total = 0;
}

ticcmd_t        emptycmd;
ticcmd_t*       I_BaseTiccmd(void)
{
    return &emptycmd;
}

//
// I_GetTime
// returns time in 1/TICRATE second tics
//
tic_t I_GetTime(void)
{
    Uint32        ticks;
    static Uint32 basetime=0;

    // milliseconds since SDL initialization
    ticks = SDL_GetTicks();

    if (!basetime)
        basetime = ticks;

    return (ticks - basetime)*TICRATE/1000;
}

/// sleeps for a while, giving CPU time to other processes
void I_Sleep(unsigned int ms)
{
  SDL_Delay(ms);
}

//
// I_Quit
//
void I_Quit (void)
{
  // prevent recursive I_Quit()
  static int quitting = 0;

  if (quitting) return;

  quitting = 1;
  //added:16-02-98: when recording a demo, should exit using 'q' key,
  //        but sometimes we forget and use 'F10'.. so save here too.
    if (demorecording)
        G_CheckDemoStatus();
    D_QuitNetGame ();
    I_ShutdownSound();
    I_ShutdownCD();
   // use this for 1.28 19990220 by Kin
    M_SaveConfig (NULL);
    I_ShutdownJoystick();
    I_ShutdownGraphics();
    I_ShutdownSystem();
    printf("\r");
    ShowEndTxt();
    exit(0);
}

void I_WaitVBL(int count)
{
    SDL_Delay(1);
}


//
// I_Error
//
extern boolean demorecording;

void I_Error (const char *error, ...)
{
    va_list     argptr;

    // Message first.
    va_start (argptr,error);
    fprintf (stderr, "Error: ");
    vfprintf (stderr,error,argptr);
    fprintf (stderr, "\n");
    va_end (argptr);

    fflush( stderr );

    // Shutdown. Here might be other errors.
    if (demorecording)
        G_CheckDemoStatus();

    D_QuitNetGame ();
    I_ShutdownJoystick();
    I_ShutdownSound();
    I_ShutdownGraphics();
    // shutdown everything else which was registered
    I_ShutdownSystem();

    exit(-1);
}
#define MAX_QUIT_FUNCS     16
typedef void (*quitfuncptr)();
static quitfuncptr quit_funcs[MAX_QUIT_FUNCS] =
               { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
               };
//
//  Adds a function to the list that need to be called by I_SystemShutdown().
//
void I_AddExitFunc(void (*func)())
{
   int c;

   for (c=0; c<MAX_QUIT_FUNCS; c++) {
      if (!quit_funcs[c]) {
         quit_funcs[c] = func;
         break;
      }
   }
}


//
//  Removes a function from the list that need to be called by
//   I_SystemShutdown().
//
void I_RemoveExitFunc(void (*func)())
{
   int c;

   for (c=0; c<MAX_QUIT_FUNCS; c++) {
      if (quit_funcs[c] == func) {
         while (c<MAX_QUIT_FUNCS-1) {
            quit_funcs[c] = quit_funcs[c+1];
            c++;
         }
         quit_funcs[MAX_QUIT_FUNCS-1] = NULL;
         break;
      }
   }
}

//
//  Closes down everything. This includes restoring the initial
//  pallete and video mode, and removing whatever mouse, keyboard, and
//  timer routines have been installed.
//
//  NOTE : Shutdown user funcs. are effectively called in reverse order.
//
void I_ShutdownSystem()
{
   int c;

   for (c=MAX_QUIT_FUNCS-1; c>=0; c--)
      if (quit_funcs[c])
         (*quit_funcs[c])();

}

uint64_t I_GetDiskFreeSpace(void)
{
#ifdef LINUX
#ifdef SOLARIS
  goto guess;

#else
  struct statfs stfs;
  if (statfs(".", &stfs) == -1)
    goto guess;

  return stfs.f_bavail * stfs.f_bsize;
#endif

#elif defined(WIN32)
  ULARGE_INTEGER free;
  if (!GetDiskFreeSpaceEx(NULL, &free, NULL, NULL))
    goto guess;

  return ((uint64_t)free.HighPart << 32) + free.LowPart;

#else
  // unknown
  goto guess;
#endif

guess:
  return MAXINT;
}

char *I_GetUserName(void)
{
  static char username[MAXPLAYERNAME];
  char  *p;

#ifdef WIN32
  DWORD i = MAXPLAYERNAME;
  int ret = GetUserName(username, &i);
  if(!ret)
    {
#endif

  if ((p = getenv("USER")) == NULL)
    if ((p = getenv("user")) == NULL)
      if ((p = getenv("USERNAME")) == NULL)
	if ((p = getenv("username")) == NULL)
	  return NULL;

  strncpy(username, p, MAXPLAYERNAME);

#ifdef WIN32
    }
#endif

  if (strcmp(username, "") == 0)
    return NULL;

  return username;
}


int  I_mkdir(const char *dirname, int unixright)
{
//[segabor]  ([WDJ] from 143beta_macosx)
#if defined(LINUX) || defined(__MACH__)
    return mkdir(dirname, unixright);
#else
    return mkdir(dirname);
#endif
}



// return free and total system memory in bytes 
uint64_t I_GetFreeMem(uint64_t *total)
{
#ifdef LINUX
  // LINUX covers all the unix-type OS's.

#ifdef FREEBSD
    struct  vmmeter sum;
    kvm_t *kd;
    struct nlist namelist[]= {
#define X_SUM   0
	{"_cnt"},
	{ NULL }
    };
    if ((kd = kvm_open(NULL, NULL, NULL, O_RDONLY, "kvm_open")) == NULL)
      goto guess;

    if (kvm_nlist(kd, namelist) != 0)
    {
	kvm_close (kd);
	goto guess;
    }
    if (kvm_read(kd,namelist[X_SUM].n_value ,&sum, sizeof(sum)) != sizeof(sum))
    {
	kvm_close (kd);
	goto guess;
    }
    kvm_close (kd);

    *total = sum.v_page_count * sum.v_page_size;
    return sum.v_free_count * sum.v_page_size;
#elif defined(SOLARIS)
    goto guess;
#else
    // Actual Linux

#define MEMINFO_FILE "/proc/meminfo"
#define MEMTOTAL "MemTotal:"
#define MEMFREE "MemFree:"

    char buf[1024];    
    char *memTag;
    uint64_t freeKBytes;
    uint64_t totalKBytes;

    int meminfo_fd = open(MEMINFO_FILE, O_RDONLY);
    int n = read(meminfo_fd, buf, 1023);
    close(meminfo_fd);
    
    if(n<0)
      goto guess;
    
    buf[n] = '\0';
    if(NULL == (memTag = strstr(buf, MEMTOTAL)))
      goto guess;
        
    memTag += sizeof(MEMTOTAL);
    totalKBytes = atoi(memTag);
    
    if(NULL == (memTag = strstr(buf, MEMFREE)))
      goto guess;
        
    memTag += sizeof(MEMFREE);
    freeKBytes = atoi(memTag);
    
    *total = totalKBytes << 10;
    return freeKBytes << 10;
#endif // Unix flavors
#elif defined(WIN32)
  // windows

  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  GlobalMemoryStatusEx(&statex);

  *total = statex.ullTotalPhys;
  return statex.ullAvailPhys;

#else
  // unknown
  goto guess;
#endif

 guess:
  // make a conservative guess
  *total = 32 << 20;
  return   32 << 20;
}
