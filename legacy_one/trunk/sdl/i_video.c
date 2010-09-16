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
// $Log: i_video.c,v $
// Revision 1.13  2004/05/16 19:11:53  hurdler
// that should fix issues some people were having in 1280x1024 mode (and now support up to 1600x1200)
//
// Revision 1.12  2002/07/01 19:59:59  metzgermeister
// *** empty log message ***
//
// Revision 1.11  2001/12/31 16:56:39  metzgermeister
// see Dec 31 log
// .
//
// Revision 1.10  2001/08/20 20:40:42  metzgermeister
// *** empty log message ***
//
// Revision 1.9  2001/05/16 22:33:35  bock
// Initial FreeBSD support.
//
// Revision 1.8  2001/04/28 14:25:03  metzgermeister
// fixed mouse and menu bug
//
// Revision 1.7  2001/04/27 13:32:14  bpereira
// no message
//
// Revision 1.6  2001/03/12 21:03:10  metzgermeister
//   * new symbols for rendererlib added in SDL
//   * console printout fixed for Linux&SDL
//   * Crash fixed in Linux SW renderer initialization
//
// Revision 1.5  2001/03/09 21:53:56  metzgermeister
// *** empty log message ***
//
// Revision 1.4  2001/02/24 13:35:23  bpereira
// no message
//
// Revision 1.3  2001/01/25 22:15:45  bpereira
// added heretic support
//
// Revision 1.2  2000/11/02 19:49:40  bpereira
// no message
//
// Revision 1.1  2000/09/10 10:56:00  metzgermeister
// clean up & made it work again
//
// Revision 1.1  2000/08/21 21:17:32  metzgermeister
// Initial import to CVS
//
//
//
// DESCRIPTION:
//      DOOM graphics stuff for SDL
//
//-----------------------------------------------------------------------------

#include <stdlib.h>

#include "SDL.h"


#include "doomdef.h"

#include "doomstat.h"
#include "i_system.h"
#include "v_video.h"
#include "m_argv.h"
#include "m_menu.h"
#include "d_main.h"
#include "s_sound.h"
#include "g_input.h"
#include "st_stuff.h"
#include "g_game.h"
#include "i_video.h"
#include "hardware/hw_main.h"
#include "hardware/hw_drv.h"
#include "console.h"
#include "command.h"
#include "hwsym_sdl.h" // For dynamic referencing of HW rendering functions
#include "ogl_sdl.h"


// maximum number of windowed modes (see windowedModes[][])
#define MAXWINMODES (8)

//Hudler: 16/10/99: added for OpenGL gamma correction
RGBA_t  gamma_correction = {0x7F7F7F7F};
extern consvar_t cv_grgammared;
extern consvar_t cv_grgammagreen;
extern consvar_t cv_grgammablue;

extern consvar_t cv_fullscreen; // for fullscreen support

static int numVidModes= 0;

static char vidModeName[33][32]; // allow 33 different modes

rendermode_t    rendermode=render_soft;
boolean highcolor = false;

// synchronize page flipping with screen refresh
// unused and for compatibilityy reason
consvar_t       cv_vidwait = {"vid_wait","1",CV_SAVE,CV_OnOff};

byte graphics_started = 0; // Is used in console.c and screen.c

// To disable fullscreen at startup; is set in VID_PrepareModeList
boolean allow_fullscreen = false;


// SDL vars

// [WDJ] appeared in 143beta_macosx without static
//   It may be the MAC version of gcc 3.3, so make it conditional on MACOS
#ifdef __MACOS__
//[segabor] !!! I had problem compiling this source with gcc 3.3
// maybe gcc 3.2 does it better
	     SDL_Surface *vidSurface=NULL;
#else
static       SDL_Surface *vidSurface=NULL;
#endif

static       SDL_Color    localPalette[256];
static       SDL_Rect   **modeList = NULL;  // fullscreen video modes
static       int firstEntry = 0; // first entry in modeList which is not bigger than 1600x1200
static       Uint8        BitsPerPixel;
const static Uint32       surfaceFlags = SDL_HWSURFACE|SDL_HWPALETTE|SDL_DOUBLEBUF;


// windowed video modes from which to choose from.
static int windowedModes[MAXWINMODES][2] = {
    {MAXVIDWIDTH /*1600*/, MAXVIDHEIGHT/*1200*/},
    {1280, 1024},
    {1024, 768},
    {800, 600},
    {640, 480},
    {512, 384},
    {400, 300},
    {320, 200}};


//
// I_StartFrame
//
void I_StartFrame(void)
{
    if(render_soft == rendermode)
    {
        if(SDL_MUSTLOCK(vidSurface))
        {
            if(SDL_LockSurface(vidSurface) < 0)
                return;
        }
    }

    return;
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit(void)
{
    /* this function intentionally left empty */
}

//
// I_FinishUpdate
//
void I_FinishUpdate(void)
{
    if(render_soft == rendermode)
    {
        if(screens[0] != vid.direct)
        {
            memcpy(vid.direct, screens[0], vid.width*vid.height*vid.bpp);
            //screens[0] = vid.direct; //FIXME: we MUST render directly into the surface
        }

        //SDL_Flip(vidSurface);
        SDL_UpdateRect(vidSurface, 0, 0, 0, 0);

        if(SDL_MUSTLOCK(vidSurface))
        {
            SDL_UnlockSurface(vidSurface);
        }
    }
    else
    {
        OglSdlFinishUpdate(cv_vidwait.value);
    }

    I_GetEvent();

    return;
}


//
// I_ReadScreen
//
void I_ReadScreen(byte* scr)
{
    if (rendermode != render_soft)
        I_Error ("I_ReadScreen: called while in non-software mode");

    memcpy (scr, screens[0], vid.width*vid.height*vid.bpp);
}



//
// I_SetPalette
//
void I_SetPalette(RGBA_t* palette)
{
    int i;

    for(i=0; i<256; i++)
    {
        localPalette[i].r = palette[i].s.red;
        localPalette[i].g = palette[i].s.green;
        localPalette[i].b = palette[i].s.blue;
    }

    SDL_SetColors(vidSurface, localPalette, 0, 256);

    return;
}


// return number of fullscreen + X11 modes
int   VID_NumModes(void)
{
    if(cv_fullscreen.value)
        return numVidModes - firstEntry;
    else
        return MAXWINMODES;
}

char  *VID_GetModeName(int modeNum)
{
    if(cv_fullscreen.value) { // fullscreen modes
        modeNum += firstEntry;
        if(modeNum >= numVidModes)
            return NULL;

        sprintf(&vidModeName[modeNum][0], "%dx%d",
                modeList[modeNum]->w,
                modeList[modeNum]->h);
    }
    else { // windowed modes
        if(modeNum > MAXWINMODES)
            return NULL;

        sprintf(&vidModeName[modeNum][0], "win %dx%d",
                windowedModes[modeNum][0],
                windowedModes[modeNum][1]);
    }
    return &vidModeName[modeNum][0];
}

int VID_GetModeForSize(int w, int h) {
    int matchMode, i;

    if(cv_fullscreen.value)
    {
        matchMode=-1;

        for(i=firstEntry; i<numVidModes; i++)
        {
            if(modeList[i]->w == w &&
               modeList[i]->h == h)
            {
                matchMode = i;
                break;
            }
        }
        if(-1 == matchMode) // use smallest mode
        {
            matchMode = numVidModes-1;
        }
        matchMode -= firstEntry;
    }
    else
    {
        matchMode=-1;

        for(i=0; i<MAXWINMODES; i++)
        {
            if(windowedModes[i][0] == w &&
               windowedModes[i][1] == h)
            {
                matchMode = i;
                break;
            }
        }

        if(-1 == matchMode) // use smallest mode
        {
            matchMode = MAXWINMODES-1;
        }
    }

    return matchMode;
}

// [smite] I see no reason to keep this function, should merge it with I_StartupGraphics
static void VID_PrepareModeList(void)
{
    int i;

    // only fullscreen needs preparation
            for(i=0; i<numVidModes; i++)
            {
                if(modeList[i]->w <= MAXVIDWIDTH &&
                   modeList[i]->h <= MAXVIDHEIGHT)
                {
                    firstEntry = i;
                    break;
                }
            }

    allow_fullscreen = true;
    return;
}

int VID_SetMode(int modeNum)
{
    doUngrabMouse();

    if(cv_fullscreen.value)
    {
        modeNum += firstEntry;

        vid.width = modeList[modeNum]->w;
        vid.height = modeList[modeNum]->h;
        vid.rowbytes = vid.width * vid.bpp;
        vid.recalc = true;

        if(render_soft == rendermode)
        {
            SDL_FreeSurface(vidSurface);
            free(vid.buffer);

            vidSurface = SDL_SetVideoMode(vid.width, vid.height, BitsPerPixel, surfaceFlags|SDL_FULLSCREEN);
            if(NULL == vidSurface)
            {
                I_Error("Could not set vidmode\n");
            }

            vid.buffer = malloc(vid.width * vid.height * vid.bpp * NUMSCREENS);

            vid.direct = vidSurface->pixels; // FIXME
        }
        else // (render_soft == rendermode)
        {
            if(!OglSdlSurface(vid.width, vid.height, cv_fullscreen.value))
            {
                I_Error("Could not set vidmode\n");
            }

        }
        vid.modenum = modeNum-firstEntry;
    }
    else //(cv_fullscreen.value)
    {
        vid.width = windowedModes[modeNum][0];
        vid.height = windowedModes[modeNum][1];
        vid.rowbytes = vid.width * vid.bpp;
        vid.recalc = true;

        if(render_soft == rendermode)
        {
            SDL_FreeSurface(vidSurface);
            free(vid.buffer);

            vidSurface = SDL_SetVideoMode(vid.width, vid.height, BitsPerPixel, surfaceFlags);

            if(NULL == vidSurface)
            {
                I_Error("Could not set vidmode\n");
            }

            vid.buffer = malloc(vid.width * vid.height * vid.bpp * NUMSCREENS);
            vid.direct = vidSurface->pixels; // FIXME
        }
        else //(render_soft == rendermode)
        {
            if(!OglSdlSurface(vid.width, vid.height, cv_fullscreen.value))
            {
                I_Error("Could not set vidmode\n");
            }
        }
        vid.modenum = modeNum;
    }

    I_StartupMouse();

    return 1;
}

void I_StartupGraphics()
{
    if(graphics_started)
        return;

    CV_RegisterVar (&cv_vidwait);

    // Get video info for screen resolutions
#ifdef __MACH__
    //[segabor]: it's ok on Mac OS X with SDL
    SDL_VideoInfo *videoInfo = SDL_GetVideoInfo();
    BitsPerPixel	= videoInfo->vfmt->BitsPerPixel;
    vid.bpp		= videoInfo->vfmt->BytesPerPixel;
    highcolor		= (vid.bpp == 2) ? true:false;
#else
    //videoInfo = SDL_GetVideoInfo();
    // even if I set vid.bpp and highscreen properly it does seem to
    // support only 8 bit  ...  strange
    // so lets force 8 bit
    BitsPerPixel = 8;

    // Set color depth; either 1=256pseudocolor or 2=hicolor
    vid.bpp = 1 /*videoInfo->vfmt->BytesPerPixel*/;
    highcolor = (vid.bpp == 2) ? true:false;
#endif    

    modeList = SDL_ListModes(NULL, SDL_FULLSCREEN|surfaceFlags);

    if(NULL == modeList)
    {
        CONS_Printf("No video modes present\n");
        return;
    }

    numVidModes=0;
    if(NULL != modeList)
    {
        while(NULL != modeList[numVidModes])
            numVidModes++;
    }
    else
        // should not happen with fullscreen modes
        numVidModes = -1;

    //CONS_Printf("Found %d Video Modes\n", numVidModes);

    VID_PrepareModeList();

    // default size for startup
    vid.width = BASEVIDWIDTH;
    vid.height = BASEVIDHEIGHT;
    vid.rowbytes = vid.width * vid.bpp;
    vid.recalc = true;

// [WDJ] To be safe, make it conditional on MACOS
#ifdef __MACOS__
    //[segabor]: Mac hack
    if(M_CheckParm("-opengl") || rendermode == render_opengl) 
#else     
    if(M_CheckParm("-opengl")) 
#endif  
    {
       rendermode = render_opengl;
       HWD.pfnInit             = hwSym("Init");
       HWD.pfnFinishUpdate     = hwSym("FinishUpdate");
       HWD.pfnDraw2DLine       = hwSym("Draw2DLine");
       HWD.pfnDrawPolygon      = hwSym("DrawPolygon");
       HWD.pfnSetBlend         = hwSym("SetBlend");
       HWD.pfnClearBuffer      = hwSym("ClearBuffer");
       HWD.pfnSetTexture       = hwSym("SetTexture");
       HWD.pfnReadRect         = hwSym("ReadRect");
       HWD.pfnGClipRect        = hwSym("GClipRect");
       HWD.pfnClearMipMapCache = hwSym("ClearMipMapCache");
       HWD.pfnSetSpecialState  = hwSym("SetSpecialState");
       HWD.pfnSetPalette       = hwSym("SetPalette");
       HWD.pfnGetTextureUsed   = hwSym("GetTextureUsed");

       HWD.pfnDrawMD2          = hwSym("DrawMD2");
       HWD.pfnSetTransform     = hwSym("SetTransform");
       HWD.pfnGetRenderVersion = hwSym("GetRenderVersion");

       // check gl renderer lib
       if (HWD.pfnGetRenderVersion() != VERSION)
       {
           I_Error ("The version of the renderer doesn't match the version of the executable\nBe sure you have installed Doom Legacy properly.\n");
       }

       vid.width = 640; // hack to make voodoo cards work in 640x480
       vid.height = 480;

       if(!OglSdlSurface(vid.width, vid.height, cv_fullscreen.value))
           rendermode = render_soft;
    }

    if(render_soft == rendermode)
    {
        vidSurface = SDL_SetVideoMode(vid.width, vid.height, BitsPerPixel, surfaceFlags);

        if(NULL == vidSurface)
        {
            CONS_Printf("Could not set vidmode\n");
            return;
        }
        vid.buffer = malloc(vid.width * vid.height * vid.bpp * NUMSCREENS);
        vid.direct = vidSurface->pixels; // FIXME
    }

    SDL_ShowCursor(0);
    doUngrabMouse();

    graphics_started = 1;

    return;
}


void I_ShutdownGraphics()
{
    // was graphics initialized anyway?
    if (!graphics_started)
        return;

    if(render_soft == rendermode)
    {
        if(NULL != vidSurface)
        {
            SDL_FreeSurface(vidSurface);
            vidSurface = NULL;
        }
    }
    else
    {
        OglSdlShutdown();
    }

    SDL_Quit();
}
