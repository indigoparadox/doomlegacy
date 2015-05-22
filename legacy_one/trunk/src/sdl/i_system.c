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
//
// Revision 1.9  2001/08/20 20:40:42  metzgermeister
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
// Revision 1.5  2000/11/02 19:49:40  bpereira
// Revision 1.4  2000/10/16 21:20:53  hurdler
//
// Revision 1.3  2000/09/26 17:58:06  metzgermeister
// I_Getkey implemented
//
// Revision 1.2  2000/09/10 10:56:00  metzgermeister
// Revision 1.1  2000/08/21 21:17:32  metzgermeister
// Initial import to CVS
//
//
// DESCRIPTION:
//   SDL system interface
//
//-----------------------------------------------------------------------------

//#define DEBUG_MOUSEMOTION
  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "SDL.h"


#ifdef LINUX
# ifdef FREEBSD
#  include <sys/param.h>
#  include <sys/mount.h>
   // meminfo
#  include <sys/types.h>
#  include <sys/sysctl.h>
# elif defined( __MACH__ )
#  include <sys/statvfs.h>
# else
#  include <sys/vfs.h>
# endif
#endif

#ifdef LMOUSE2
#include <termios.h>
#endif

#include <libgen.h>
  // dirname function

#include "doomincl.h"
#include "m_misc.h"
#include "screen.h"
#include "i_video.h"
#include "i_sound.h"
#include "i_system.h"

#include "d_net.h"
#include "g_game.h"
#include "g_input.h"

#include "keys.h"
#include "i_joy.h"
#include "m_argv.h"

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

static int vid_center_x = 100;
static int vid_center_y = 100;
static int mouse_x_min = 25;
static int mouse_x_max = 175;
static int mouse_y_min = 25;
static int mouse_y_max = 175;
static int lastmousex = 0;
static int lastmousey = 0;

#ifdef LMOUSE2
extern void I_GetMouse2Event(void);
#endif

// current modifier key status
boolean shiftdown = false;
boolean altdown = false;


void I_GetEvent(void)
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
	      event.type = ev_mouse;
	      event.data1 = 0;
	      // [WDJ] 8/2012 Some problems with Absolute mouse motion in OpenBSD.
	      // Could not predict which would work best for a particular port,
	      // so both are here, selected from mouse menu.
	      if( cv_mouse_motion.value )
	      {
		  // Relative mouse motion interface.
		  // Seems to be used by prboom and some other SDL Doom ports.
		  // SDL 2001 docs: Windows and Linux, otherwise don't know.
		  // Requires that SDL xrel and yrel report motion even when
		  // abs mouse position is limited at window border by grabinput.
		  // Linux: rel motion continues even when abs motion stopped by grabinput.
		  // OpenBSD: seems to work except when grabinput=0.
#ifdef DEBUG_MOUSEMOTION
		  fprintf(stderr, "Mouse %i,%i, rel %i,%i\n",
		      inputEvent.motion.x, inputEvent.motion.y,
		      inputEvent.motion.xrel, inputEvent.motion.yrel);
#endif
		  // y is negated because screen + is down, but map + is up.
		  event.data2 = inputEvent.motion.xrel << 2;
		  event.data3 = - (inputEvent.motion.yrel << 2);
	      }
	      else
	      {
	          // Absolute mouse motion interface.  Default.
		  // Linux: works in all combinations.
		  // Windows: works, untested on newer
		  // OpenBSD: works, except that when grabinput=0 mouse
		  // cannot escape window.
#ifdef DEBUG_MOUSEMOTION
		  fprintf(stderr, "Mouse %i,%i,  old %i,%i,  rel %i,%i\n",
		      inputEvent.motion.x, inputEvent.motion.y,
		      lastmousex, lastmousey,
		      inputEvent.motion.x - lastmousex, inputEvent.motion.y - lastmousey);
#endif
		  // First calc relative motion using lastmouse,
		  // so can save lastmouse before WarpMouse test
		  event.data2 = (inputEvent.motion.x - lastmousex) << 2;
		  lastmousex = inputEvent.motion.x;
		  // y is negated because screen + is down, but map + is up. 
		  event.data3 = (lastmousey - inputEvent.motion.y) << 2;
		  lastmousey = inputEvent.motion.y;
	      }
#ifdef DEBUG_WINDOWED
	      // DEBUG_WINDOWED blocks grabinput effects to get easy access to
	      // debugging window, so it always needs WarpMouse.
#else
	      // With Relative mouse motion and input grabbed,
	      // SDL will limit range with (xrel, yrel) still working
	      // Known to work on Linux, OpenBSD, and Windows.
	      // Absolute mouse motion requires WarpMouse centering always.
	      // Keyboard will be affected by grabinput, independently of this.
	      if( (cv_mouse_motion.value==0) || ! cv_grabinput.value )
#endif
	      {
		  static byte lastmouse_warp = 0;
		  // If the event is from warping the pointer back to middle
		  // of the screen then ignore it.  Not often, 45 degree turn.
		  if (lastmouse_warp
		      && (inputEvent.motion.x == vid_center_x)
		      && (inputEvent.motion.y == vid_center_y) )
		  {
		      lastmouse_warp = 0;
		      break;  // skip PostEvent
		  }

		  // Warp the pointer back to the middle of the window
		  //  or we cannot move any further when it reaches a border.
		  if ((inputEvent.motion.x < mouse_x_min) ||
		      (inputEvent.motion.y < mouse_y_min) ||
		      (inputEvent.motion.x > mouse_x_max) ||
		      (inputEvent.motion.y > mouse_y_max)   )
		  {
		      // Warp the pointer back to the middle of the window
		      SDL_WarpMouse(vid_center_x, vid_center_y);
		      // this issues a mouse event that needs to be ignored
		      lastmouse_warp = 1;
		  }
	      }
	      // issue mouse event
	      D_PostEvent(&event);
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

// [WDJ] 8/2012 Grab mouse re-enabled as option menu item.

static void doGrabMouse(void)
{
  if( cv_grabinput.value && !devparm )
  {
      if(SDL_GRAB_OFF == SDL_WM_GrabInput(SDL_GRAB_QUERY))
      {
#ifdef DEBUG_WINDOWED
	 // do not grab so can use debugger
#else
	 SDL_WM_GrabInput(SDL_GRAB_ON);
#endif
      }
  }
}

void doUngrabMouse(void)
{
  if(SDL_GRAB_ON == SDL_WM_GrabInput(SDL_GRAB_QUERY))
  {
      SDL_WM_GrabInput(SDL_GRAB_OFF);
  }
}

// Called on video mode change, usemouse change, mousemotion change,
// and game paused.
//   play_mode : enable mouse containment during play
void I_StartupMouse( boolean play_mode )
{
    vid_center_x = vid.width >> 1;
    vid_center_y = vid.height >> 1;
    lastmousex = vid_center_x;
    lastmousey = vid_center_y;
    if( cv_usemouse.value && play_mode )
    {
        // Enable mouse containment during play.
        SDL_Event inputEvent;
        // warp to center
        SDL_WarpMouse(vid_center_x, vid_center_y);
        // remove the mouse event by reading the queue
        SDL_PollEvent(&inputEvent);

        // Guard band at window border: 20%=51, 25%=64, 30%=76
        mouse_x_min = (vid.width * 64) >> 8;
        mouse_x_max = vid.width - mouse_x_min;
        mouse_y_min = (vid.height * 64) >> 8;
        mouse_y_max = vid.height - mouse_y_min;

        doGrabMouse();
    }
    else
    {
        // Disable Guard band.
        mouse_x_min = -1;
        mouse_x_max = 20000;
        mouse_y_min = -1;
        mouse_y_max = 20000;

        doUngrabMouse();
    }
    return;
}


/// Initialize joysticks and print information.
void I_JoystickInit(void)
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
      if (devparm || verbose > 1)
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
void I_ShutdownJoystick(void)
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
void I_SysInit(void)
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
void I_OsPolling(void)
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
void I_ShutdownMouse2(void) {
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

// sleeps for a while, giving CPU time to other processes
// in milleseconds
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
    D_Quit_NetGame ();
    I_ShutdownSound();
#ifdef CDMUS
    I_ShutdownCD();
#endif
   // use this for 1.28 19990220 by Kin
    M_SaveConfig (NULL);
    I_ShutdownJoystick();
    I_ShutdownGraphics();
    I_ShutdownSystem();
    printf("\r");
    ShowEndTxt();
    exit(0);
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

    D_Quit_NetGame ();
    I_ShutdownJoystick();
    I_ShutdownSound();
    I_Sleep( 3000 );  // to see some messages
    I_ShutdownGraphics();
    // shutdown everything else which was registered
    I_ShutdownSystem();

    exit(-1);
}

#define MAX_QUIT_FUNCS     16
typedef void (*quitfuncptr)(void);
static quitfuncptr quit_funcs[MAX_QUIT_FUNCS] =
               { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                 NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
               };
//
//  Adds a function to the list that need to be called by I_SystemShutdown().
//
void I_AddExitFunc(void (*func)(void))
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
void I_RemoveExitFunc(void (*func)(void))
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
void I_ShutdownSystem(void)
{
   int c;

   for (c=MAX_QUIT_FUNCS-1; c>=0; c--)
      if (quit_funcs[c])
         (*quit_funcs[c])();

}

uint64_t I_GetDiskFreeSpace(void)
{
#ifdef LINUX
# ifdef SOLARIS
  goto guess;

# elif defined( __MACH__ )
  struct statvfs stfs;
  if (statvfs(".", &stfs) == -1)
    goto guess;

  return (uint64_t) (stfs.f_bavail * stfs.f_bsize);
# else
  struct statfs stfs;
  if (statfs(".", &stfs) == -1)
    goto guess;

  return (uint64_t) (stfs.f_bavail * stfs.f_bsize);
# endif

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


// Get the directory of this program.
//   defdir: the current directory
//   dirbuf: a buffer of length MAX_WADPATH, 
// Return true when success, dirbuf contains the directory.
boolean I_Get_Prog_Dir( char * defdir, /*OUT*/ char * dirbuf )
{
    int  len;
    char * dnp;

#ifdef LINUX
# ifdef FREEBSD
    len = readlink( "/proc/curproc/file", dirbuf, MAX_WADPATH-1 );
    if( len > 1 )
    {
        dirbuf[len] = 0;  // readlink does not terminate string
        goto got_path;
    }
# if 0
    // Sysctl to get program path.
    {
        // FIXME
        int mib[4];
        mib[0] = CTL_KERN;
        mib[1] = KERN_PROC;
        mib[2] = KERN_PROC_PATHNAME;
        mib[3] = -1;
        sysctl(mib, 4, dirbuf, MAX_WADPATH-1, NULL, 0);
    }
# endif
# elif defined( SOLARIS )
    len = readlink( "/proc/self/path/a.out", dirbuf, MAX_WADPATH-1 );
    if( len > 1 )
    {
        dirbuf[len] = 0;  // readlink does not terminate string
        goto got_path;
    }
#  if 0
    // [WDJ] Am missing a few details
    getexecname();
#  endif
# elif defined( __MACH__ )
#  if 0
    uint32_t  bufsize = MAX_WADPATH-1;
    // [WDJ] Am missing a few details
    if( _NSGetExecutablePath( dirbuf, & bufsize ) == 0 )   goto got_path;
#  endif
# else
    // Linux
    len = readlink( "/proc/self/exe", dirbuf, MAX_WADPATH-1 );
    if( len > 1 )
    {
        dirbuf[len] = 0;  // readlink does not terminate string
        goto got_path;
    }
# endif
#else
# ifdef WIN32
//    if(  )
    {
        // MS-Docs say Windows XP, but MinGW does not guard it.
        len = GetModuleFileName( NULL, dirbuf, MAX_WADPATH-1 );
        if( len > 1 )
        {
            dirbuf[len] = 0;  // does not terminate string ??
            goto got_path;
        }
    }
# endif
#endif
    // The argv[0] method
    char * arg0p = myargv[0];
//    GenPrintf(EMSG_debug, "argv[0]=%s\n", arg0p );
#ifdef LINUX
    // Linux, FreeBSD, Mac
    if( arg0p[0] == '/' )
#else
    // Windows, DOS, OS2
    if( arg0p[0] == '\\' )
#endif
    {
        // argv[0] is an absolute path
        strncpy( dirbuf, arg0p, MAX_WADPATH-1 );
        dirbuf[MAX_WADPATH-1] = 0;
        goto got_path;
    }
#ifdef LINUX
    // Linux, FreeBSD, Mac
    else if( strchr( arg0p, '/' ) )
#else
    // Windows, DOS, OS2
    else if( strchr( arg0p, '/' ) || strchr( arg0p, '\\' ) )
#endif
    {
        // argv[0] is relative to current dir
        if( defdir )
        {
            cat_filename( dirbuf, defdir, arg0p );
            goto got_path;
        }
    }
    goto failed;
   
got_path:
    // Get only the directory name
    dnp = dirname( dirbuf );
    if( dnp == NULL )  goto failed;
    if( dnp != dirbuf )
    {
        cat_filename( dirbuf, "", dnp );
    }
    return true;

failed:
    dirbuf[0] = 0;
    return false;
}



int  I_mkdir(const char * new_dirname, int unixright)
{
//[segabor]  ([WDJ] from 143beta_macosx)
#if defined(LINUX) || defined(__MACH__)
    return mkdir(new_dirname, unixright);
#else
    return mkdir(new_dirname);
#endif
}



// return free and total system memory in bytes 
uint64_t I_GetFreeMem(uint64_t *total)
{
#ifdef LINUX
  // LINUX covers all the unix-type OS's.

#ifdef FREEBSD
    unsigned page_count, free_count, pagesize;
    size_t len = sizeof(unsigned);
    if (sysctlbyname("vm.stats.vm.v_page_count", &page_count, &len, NULL, 0))
      goto guess;
    if (sysctlbyname("vm.stats.vm.v_free_count", &free_count, &len, NULL, 0))
      goto guess;
    if (sysctlbyname("hw.pagesize", &pagesize, &len, NULL, 0))
      goto guess;
    *total = (uint64_t)page_count * pagesize;
    return (uint64_t)free_count * pagesize;
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
#endif

 guess:
    // make a conservative guess
    *total = (32 << 20) + 0x01;  // guess indicator
    return   0;
#elif defined(WIN32)
  // windows
#if defined(WIN_LARGE_MEM) && defined( _WIN32_WINNT ) && (_WIN32_WINNT >= 0x0500)
  // large memory status, only in newer libraries
  MEMORYSTATUSEX statex;
  statex.dwLength = sizeof(statex);
  GlobalMemoryStatusEx(&statex);
  *total = statex.ullTotalPhys;
  return statex.ullAvailPhys;
#else
  // older memory status
  MEMORYSTATUS statex;
  statex.dwLength = sizeof(statex);
  GlobalMemoryStatus(&statex);
  *total = statex.dwTotalPhys;
  return statex.dwAvailPhys;
#endif

#else
  // unknown
  // make a conservative guess
  *total = (32 << 20) + 0x01;  // guess indicator
  return   0;
#endif

}
