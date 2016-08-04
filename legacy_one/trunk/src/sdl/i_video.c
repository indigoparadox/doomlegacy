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
//
// Revision 1.11  2001/12/31 16:56:39  metzgermeister
// see Dec 31 log
//
// Revision 1.10  2001/08/20 20:40:42  metzgermeister
//
// Revision 1.9  2001/05/16 22:33:35  bock
// Initial FreeBSD support.
//
// Revision 1.8  2001/04/28 14:25:03  metzgermeister
// fixed mouse and menu bug
//
// Revision 1.7  2001/04/27 13:32:14  bpereira
//
// Revision 1.6  2001/03/12 21:03:10  metzgermeister
//   * new symbols for rendererlib added in SDL
//   * console printout fixed for Linux&SDL
//   * Crash fixed in Linux SW renderer initialization
//
// Revision 1.5  2001/03/09 21:53:56  metzgermeister
//
// Revision 1.4  2001/02/24 13:35:23  bpereira
//
// Revision 1.3  2001/01/25 22:15:45  bpereira
// added heretic support
//
// Revision 1.2  2000/11/02 19:49:40  bpereira
// Revision 1.1  2000/09/10 10:56:00  metzgermeister
// Revision 1.1  2000/08/21 21:17:32  metzgermeister
// Initial import to CVS
//
//
// DESCRIPTION:
//      DOOM graphics stuff for SDL
//
//-----------------------------------------------------------------------------

// Debugging unfinished MAC_SDL
//#define DEBUG_MAC  1

//#define TESTBPP
#ifdef TESTBPP
// [WDJ] Test drawing in a testbpp mode, using native mode conversion.
static int testbpp = 0;
#endif

#include <stdlib.h>

#include <SDL.h>

#include "doomincl.h"
#include "doomstat.h"

#include "i_system.h"
#include "i_video.h"
#include "v_video.h"
  // mode_fullscreen, etc
#include "m_argv.h"
#include "m_menu.h"
#include "d_main.h"
#include "s_sound.h"
#include "g_input.h"
#include "st_stuff.h"
#include "g_game.h"
#include "hardware/hw_main.h"
#include "hardware/hw_drv.h"
#include "console.h"
#include "hwsym_sdl.h" // For dynamic referencing of HW rendering functions
#include "ogl_sdl.h"


//Hudler: 16/10/99: added for OpenGL gamma correction
RGBA_t  gamma_correction = {0x7F7F7F7F};

// SDL vars

// [WDJ] appeared in 143beta_macosx without static
//   It may be the MAC version of gcc 3.3, so make it conditional
#if defined(__APPLE__) && defined(__GNUC__) && (__GNUC__ == 3) && (__GNUC_MINOR__ == 3)
//[segabor] !!! I had problem compiling this source with gcc 3.3
// maybe gcc 3.2 does it better
	SDL_Surface *vidSurface=NULL;
#else
static  SDL_Surface *vidSurface=NULL;
#endif

static  SDL_Color    localPalette[256];

// Video mode list
static  int   numVidModes= 0;
static  char  vidModeName[33][32]; // allow 33 different modes
// Fullscreen modelist
// modeList is not our memory to manage, do not free
static  SDL_Rect   **modeList = NULL;  // fullscreen video modes
static  int        firstEntry = 0; // first entry in modeList which is not bigger than 1600x1200
static  byte       request_bitpp = 0;  // with modelist
static  byte       request_NULL = 0;  // with modelist

#ifdef __MACOSX__
// SDL_DOUBLEBUF is unsupported for Mac OS X
const static Uint32  surfaceFlags = SDL_SWSURFACE|SDL_HWPALETTE;
// With SDL 1.2.6 there is an experimental software flipping that is
// accessed using SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_FULLSCREEN
const static Uint32  surfaceFlags_fullscreen = SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_FULLSCREEN|SDL_HWPALETTE;
#else
#if 1
// NO DOUBLEBUF, as we already draw to buffer
const static Uint32  surfaceFlags = SDL_HWSURFACE|SDL_HWPALETTE;
const static Uint32  surfaceFlags_fullscreen = SDL_HWSURFACE|SDL_HWPALETTE|SDL_FULLSCREEN;
#else
// DOUBLEBUF
const static Uint32  surfaceFlags = SDL_HWSURFACE|SDL_HWPALETTE|SDL_DOUBLEBUF;
const static Uint32  surfaceFlags_fullscreen = SDL_HWSURFACE|SDL_HWPALETTE|SDL_DOUBLEBUF|SDL_FULLSCREEN;
#endif
#endif


// maximum number of windowed modes
#define MAXWINMODES (8)
// windowed video modes from which to choose from.
static int windowedModes[MAXWINMODES+1][2] = {
   // hidden from display
    {INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT},  // initial mode
   // public  1..
    {MAXVIDWIDTH /*1600*/, MAXVIDHEIGHT/*1200*/},
    {1280, 1024},
    {1024, 768},
    {800, 600},
    {640, 480},
    {512, 384},
    {400, 300},
    {320, 200}
};


//
// I_StartFrame
//
void I_StartFrame(void)
{
    // no longer lock, no more assumed direct access
#if 0 
    if(render_soft == rendermode)
    {
        if(SDL_MUSTLOCK(vidSurface))
        {
            if(SDL_LockSurface(vidSurface) < 0)
                return;
        }
    }
#endif

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
        // [WDJ] Only lock during transfer itself.  The only access
        // to vid.direct is in this routine.
        if(SDL_MUSTLOCK(vidSurface))
            if(SDL_LockSurface(vidSurface) < 0)
                return;

#ifdef TESTBPP
        // [WDJ] To test drawing in 15bpp, 16bpp, 24bpp, convert the
	// screen drawn in the testbpp mode to the native bpp mode.
        if( testbpp )
        {
	    byte * vidmem = vid.direct;
	    byte * src = vid.display;
	    int h = vid.height;
	    while( h-- )
	    {
	        int w=vid.width;
	        switch( testbpp )
	        {
		 case 15:
	          {
		    uint32_t * v32 = (uint32_t*) vidmem;
		    uint16_t * s16 = (uint16_t*) src;
		    while( w--)
		    {
		        *v32 = ((*s16&0x7C00)<<9)|((*s16&0x03E0)<<6)|((*s16&0x001F)<<3);
		        v32++;
		        s16++;
		    }
		  }
		  break;
		 case 16:
	          {
		    uint32_t * v32 = (uint32_t*) vidmem;
		    uint16_t * s16 = (uint16_t*) src;
		    while( w--)
		    {
		        *v32 = ((*s16&0xF800)<<8)|((*s16&0x07E0)<<5)|((*s16&0x001F)<<3);
		        v32++;
		        s16++;
		    }
		  }
		  break;
		 case 24:
	          {
		    byte* v = vidmem;
		    byte* s = src;
		    while( w--)
		    {
		        *(uint16_t*)v = *(uint16_t*)s;  // g, b
		        v[2] = s[2];  // r
		        v+=4;
		        s+=3;
		    }
		  }
		  break;
		 case 32:
		  memcpy( vidmem, src, vid.widthbytes );
		  break;
		}
	        src += vid.ybytes;
	        vidmem += vid.direct_rowbytes;
	    }
	}
        else
#endif
       
	// [WDJ] SDL Spec says that you can directly read and write the surface
	// while it is locked.
        if(vid.display != vid.direct)
        {
#if 0	   
	    VID_BlitLinearScreen( vid.display, vid.direct, vid.widthbytes, vid.height, vid.ybytes, vid.direct_rowbytes);
#else
	    if( (vid.widthbytes == vid.direct_rowbytes) && (vid.ybytes == vid.direct_rowbytes))
	    {
	        // fast, copy entire buffer at once
	        memcpy(vid.direct, vid.display, vid.direct_size);
	        //screens[0] = vid.direct; //FIXME: we MUST render directly into the surface
	    }
	    else
	    {
	        // [WDJ] padded video buffer (Mac)
	        // Some cards use the padded space, so DO NOT STOMP ON IT.
	        int h = vid.height;
	        byte * vidmem = vid.direct;
	        byte * src = vid.display;
	        while( h-- )
	        {
		    memcpy(vidmem, src, vid.widthbytes);  // width limited
		    vidmem += vid.direct_rowbytes;
		    src += vid.ybytes;
		}
	    }
#endif
        }

        if(SDL_MUSTLOCK(vidSurface))
        {
            SDL_UnlockSurface(vidSurface);
        }
        // If page flip involves changing vid.display, then must change screens[0] too
        // [WDJ] SDL spec says to not call UpdateRect while vidSurface is locked
        //SDL_Flip(vidSurface);
#ifdef __MACOSX__
        // Setup Flip of DOUBLEBUF
        SDL_Flip(vidSurface);
	// Hardware that does not support DOUBLEBUF does
        // SDL_UpdateRect(vidSurface, 0, 0, 0, 0);
#else
        SDL_UpdateRect(vidSurface, 0, 0, 0, 0);
#endif
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
// Screen to screen copy
void I_ReadScreen(byte* scr)
{
    if (rendermode != render_soft)
        I_Error ("I_ReadScreen: called while in non-software mode");

#if 0	   
    VID_BlitLinearScreen( src, vid.display, vid.widthbytes, vid.height, vid.ybytes, vid.ybytes);
#else
    if( vid.widthbytes == vid.ybytes )
    {
        // fast, copy entire buffer at once
        memcpy (scr, vid.display, vid.screen_size);
    }
    else
    {
        // [WDJ] padded video buffer (Mac)
        int h = vid.height;
        byte * vidmem = vid.display;
        while( h-- )
        {
	    memcpy(scr, vidmem, vid.widthbytes);
	    vidmem += vid.ybytes;
	    scr += vid.ybytes;
	}
    }
#endif
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

#if defined(MAC_SDL) && defined( DEBUG_MAC )
    if( ! SDL_SetColors(vidSurface, localPalette, 0, 256) )
    {
        GenPrintf( EMSG_error,"Error: SDL_SetColors failed to set all colors\n");
    }
#else
    SDL_SetColors(vidSurface, localPalette, 0, 256);
#endif

    return;
}


// modetype is of modetype_e
range_t  VID_ModeRange( byte modetype )
{
    range_t  mrange = { 1, 1 };  // first is always 1
    mrange.last = (modetype == MODE_fullscreen) ?
     numVidModes - firstEntry  // fullscreen
     : MAXWINMODES;  // windows
    return mrange;
}

char * VID_GetModeName( modenum_t modenum )
{
    if( modenum.modetype == MODE_fullscreen )
    {
        // fullscreen modes  1..
        int mi = modenum.index - 1 + firstEntry;
        if(mi >= numVidModes)   goto fail;

        sprintf(&vidModeName[modenum.index][0], "%dx%d",
                modeList[mi]->w,
                modeList[mi]->h);
    }
    else
    {
        // windowed modes  1.., sometimes 0
        if(modenum.index > MAXWINMODES)   goto fail;

        sprintf(&vidModeName[modenum.index][0], "win %dx%d",
                windowedModes[modenum.index][0],
                windowedModes[modenum.index][1]);
    }
    return &vidModeName[modenum.index][0];
fail:
    return NULL;
}

// rmodetype is of modetype_e
// Returns MODE_NOP when none found
modenum_t  VID_GetModeForSize( int rw, int rh, byte rmodetype )
{
    modenum_t  modenum = { MODE_NOP, 0 };
    int bestdist = MAXINT;
    int best, tdist, i;

    if( rmodetype == MODE_fullscreen )
    {
        if( numVidModes == 0 )  goto done;
        best = numVidModes-1;  // default is smallest mode

        // search SDL modelist
        for(i=firstEntry; i<numVidModes; i++)
        {
	    tdist = abs(modeList[i]->w - rw) + abs(modeList[i]->h - rh);
	    // find closest dist
	    if( bestdist > tdist )
	    {
	        bestdist = tdist;
	        best = i;
	        if( tdist == 0 )  break;   // found exact match
	    }
        }
        modenum.index = best - firstEntry + 1;  // 1..
    }
    else
    {
        best = MAXWINMODES;  // default is smallest mode

        // window mode index returned 1..
        for(i=1; i<=MAXWINMODES; i++)
        {
	    tdist = abs(windowedModes[i][0] - rw) + abs(windowedModes[i][1] - rh);
	    // find closest dist
	    if( bestdist > tdist )
	    {
	        bestdist = tdist;
	        best = i;
	        if( tdist == 0 )  break;   // found exact match
	    }
        }
        modenum.index = best; // 1..
    }
    modenum.modetype = rmodetype;
done:
    return modenum;
}


// Set video mode and vidSurface, with verbose
static void  VID_SetMode_vid( int req_width, int req_height, int reqflags )
{
    int cbpp = SDL_VideoModeOK(req_width, req_height, request_bitpp, reqflags);
    if( cbpp == 0 )
        return; // SetMode would have failed, keep current buffers

    if(vidSurface)
    {
        SDL_FreeSurface(vidSurface);
        vidSurface = NULL;
    }
    free(vid.buffer); // was malloc
    vid.display = NULL;
    vid.buffer = NULL;
    vid.direct = NULL;
    vid.width = req_width;
    vid.height = req_height;
   
    if( verbose>1 )
    {
        GenPrintf( EMSG_ver,"SDL_SetVideoMode(%i,%i,%i,0x%X)  %s\n",
		vid.width, vid.height, request_bitpp, reqflags,
		(reqflags&SDL_FULLSCREEN)?"Fullscreen":"Window");
    }

    vidSurface = SDL_SetVideoMode(vid.width, vid.height, request_bitpp, reqflags);
    if(vidSurface == NULL)
        return;  // Modes were prechecked, SDL should not fail.
 
    if( verbose )
    {
        int32_t vflags = vidSurface->flags;
        GenPrintf( EMSG_ver,"  Got %ix%i, %i bpp, %i bytes\n",
		vidSurface->w, vidSurface->h,
		vidSurface->format->BitsPerPixel, vidSurface->format->BytesPerPixel );
        GenPrintf( EMSG_ver,"  HW-surface= %x, HW-palette= %x, HW-accel= %x, Doublebuf= %x, Async= %x \n",
		vflags&SDL_HWSURFACE, vflags&SDL_HWPALETTE, vflags&SDL_HWACCEL, vflags&SDL_DOUBLEBUF, vflags&SDL_ASYNCBLIT );
        if(SDL_MUSTLOCK(vidSurface))
	    GenPrintf( EMSG_ver,"  Notice: MUSTLOCK video surface\n" );
    }
    if( vidSurface->w != vid.width || vidSurface->h != vid.height )
    {
        GenPrintf( EMSG_ver,"  Adapting to VideoMode: requested %ix%i, got %ix%i\n",
		vid.width, vid.height,
		vidSurface->w, vidSurface->h );
        vid.width = vidSurface->w;
        vid.height = vidSurface->h;
    }
    if( vidSurface->format->BitsPerPixel != request_bitpp )
    {
        GenPrintf( EMSG_ver,"  Notice: requested %i bpp, got %i bpp\n",
		request_bitpp, vidSurface->format->BitsPerPixel );
    }

    vid.bitpp = vidSurface->format->BitsPerPixel;
    vid.bytepp = vidSurface->format->BytesPerPixel;

    // The video buffer might be padded to power of 2, for some modes (Mac)
    vid.direct_rowbytes = vidSurface->pitch; // correct, even on Mac
    vid.direct_size = vidSurface->pitch * vid.height; // correct, even on Mac
    vid.direct = vidSurface->pixels;
#ifdef TESTBPP
    if( testbpp )
    {
        // [WDJ] Force the testbpp drawing mode
        vid.bitpp = testbpp;
        switch( testbpp )
        {
	 case 15:
	 case 16:
	   vid.bytepp = 2;
	   break;
	 case 24:
	   vid.bytepp = 3;
	   break;
	 case 32:
	   vid.bytepp = 4;
	   break;
	}
    }
#endif
#if 1
 // normal
    // Because we have to copy by row anyway, buffer can be normal
    // Have option to change this for special cases,
    // most code uses vid.ybytes now, and is padded video safe.
    vid.ybytes = vid.width * vid.bytepp;
    vid.screen_size = vid.ybytes * vid.height;
#else
 // DEBUG padded video buffer code
    vid.ybytes = vid.width * vid.bytepp + 8;  // force odd size
    vid.screen_size = vid.ybytes * vid.height;
#endif
    // display is buffer
    vid.buffer = malloc(vid.screen_size * NUMSCREENS);
    vid.display = vid.buffer;
    vid.screen1 = vid.buffer + vid.screen_size;
    vid.recalc = true;
}
 

// SDL version of VID_SetMode
// Returns FAIL_end, FAIL_create, of status_return_e, 1 on success;
int VID_SetMode(modenum_t modenum)
{
    int req_width, req_height;
    boolean set_fullscreen = (modenum.modetype == MODE_fullscreen);

    GenPrintf( EMSG_info, "VID_SetMode(%s,%i)\n",
	       modetype_string[modenum.modetype], modenum.index);

    I_UngrabMouse();

    vid.recalc = true;

    if( set_fullscreen )
    {
        // fullscreen
        int mi = modenum.index - 1 + firstEntry;
        req_width = modeList[mi]->w;
        req_height = modeList[mi]->h;

        if(render_soft == rendermode)
        {
	    VID_SetMode_vid(req_width, req_height, surfaceFlags_fullscreen);  // fullscreen
	    if( vidSurface == NULL )  goto fail;
        }
        else // (render_soft == rendermode)
        {
            if(!OglSdlSurface(req_width, req_height, true))
	        goto fail;
        }
    }
    else
    {
        // not fullscreen, window, 1..
	// modenum == 0 is INITIAL_WINDOW_WIDTH
        int mi = modenum.index;
        req_width = windowedModes[mi][0];
        req_height = windowedModes[mi][1];

        if(render_soft == rendermode)
        {
	    VID_SetMode_vid( req_width, req_height, surfaceFlags );  // window
            if(vidSurface == NULL)  goto fail;
        }
        else //(render_soft == rendermode)
        {
            if(!OglSdlSurface(req_width, req_height, 0))
	        goto fail;
        }
    }
    vid.modenum = modenum;
    vid.fullscreen = set_fullscreen;
    vid.widthbytes = vid.width * vid.bytepp;

    I_StartupMouse( false );

#if defined(MAC_SDL) && defined( DEBUG_MAC )
    SDL_Delay( 2 * 1000 );  // [WDJ] DEBUG: to see if errors are due to startup or activity
#endif
    return 1;

fail:
    I_Error("VID_SetMode failed to provide display\n");
    return  FAIL_create;
}


// Voodoo card has video switch, produces fullscreen 3d graphics,
// and we cannot use window mode with it.
boolean  have_voodoo = false;

// Have to determine how to detect a voodoo card
// If anyone ever tries a voodoo card again, they will have to fix this.
static boolean detect_voodoo( void )
{
    char vb[1024];
    SDL_VideoDriverName( vb, 1022 );
    if( strstr( "Voodoo", vb ) != NULL )
       have_voodoo = true;
    return have_voodoo;
}


// Initialize the graphics system, with a initial window.
void I_StartupGraphics( void )
{
    modenum_t  initialmode = {MODE_window,0};  // the initial mode
    // pre-init by V_Init_VideoControl

    request_bitpp = 8;
   
    graphics_state = VGS_startup;
    if( VID_SetMode( initialmode ) <= 0 )
       goto abort_error;

    SDL_ShowCursor(SDL_DISABLE);
    I_StartupMouse( false );
//    I_UngrabMouse();

    graphics_state = VGS_active;
    return;

abort_error:
    // cannot return without a display screen
    I_Error("StartupGraphics Abort\n");
}


// Called to start rendering graphic screen according to the request switches.
// Fullscreen modes are possible.
void I_RequestFullGraphics( byte select_fullscreen )
{
    SDL_PixelFormat    req_format;
    char * req_errmsg = NULL;
    byte  alt_request_bitpp = 0;

    // Get video info for screen resolutions
    // even if I set vid.bytepp and highscreen properly it does seem to
    // support only 8 bit  ...  strange
    // so lets force 8 bit, default
    req_format.BitsPerPixel = 8;
    req_format.BytesPerPixel = 0;
    vid.bitpp = 8;
    // Set color depth; either 1=256pseudocolor or 2=hicolor
    vid.bytepp = 1;

    modeList = SDL_ListModes(NULL, SDL_FULLSCREEN|surfaceFlags);

    // Get and report video info
    const SDL_VideoInfo * videoInfo = (const SDL_VideoInfo *) SDL_GetVideoInfo();
    if( videoInfo )
    {
        if( verbose )
        {
	    GenPrintf( EMSG_ver,"SDL video info = { %i bits, %i bytes }\n",
		videoInfo->vfmt->BitsPerPixel, videoInfo->vfmt->BytesPerPixel );
	    if( verbose > 1 )
            {
	      GenPrintf( EMSG_ver," HW_surfaces= %i, blit_hw= %i, blit_sw = %i\n",
		videoInfo->hw_available, videoInfo->blit_hw, videoInfo->blit_sw );
	      GenPrintf( EMSG_ver," video_mem= %i K\n",
		videoInfo->video_mem );
	    }
	}
        if( req_drawmode == REQ_native )
        {
	    vid.bitpp  = videoInfo->vfmt->BitsPerPixel;
	    vid.bytepp = videoInfo->vfmt->BytesPerPixel;
	    if( V_CanDraw( vid.bitpp ))
	    {
	        request_bitpp = vid.bitpp;
	        goto get_modelist;
	    }
	    // Use 8 bit and let SDL do the palette lookup.
	    if( verbose )
	        GenPrintf( EMSG_ver,"Native %i bpp rejected\n", vid.bitpp );
	}
    }
    else
    {
        GenPrintf( EMSG_info,"No SDL video info, use default\n" );
    }

    switch(req_drawmode)
    {
     case REQ_specific:
       request_bitpp = req_bitpp;
       break;
     case REQ_highcolor:
       req_errmsg = "highcolor";
       request_bitpp = 15;
       alt_request_bitpp = 16;
       if( vid.bitpp == 16 )  request_bitpp = 16;  // native preference
       break;
     case REQ_truecolor:
       req_errmsg = "truecolor";
       request_bitpp = 24;
       alt_request_bitpp = 32;
       if( vid.bitpp == 32 )  request_bitpp = 32;  // native preference
       break;
     default:
       request_bitpp = 8;
       break;
    }

#ifdef TESTBPP
    // [WDJ] Detect testbpp flag
    // Requested bpp will succeed, driver will convert drawn screen to native bpp.
    testbpp = 0;
    if( M_CheckParm( "-testbpp" ))
    {
        if( request_bitpp == 8 )
	   I_Error( "Invalid for SDL port driver: -bpp 8 -testbpp" );
        testbpp = request_bitpp;
        request_bitpp = 32;  // native mode
    }
#endif

get_modelist:
    // try the requested bpp, then alt, then 8bpp
    while(request_bitpp)
    {
        req_format.BitsPerPixel = request_bitpp;
        modeList = SDL_ListModes(&req_format, surfaceFlags_fullscreen);
        if( modeList )  goto found_modes;

        if( request_bitpp == 8 )  break;
        if(req_drawmode == REQ_specific)
        {
	   GenPrintf( EMSG_info,"No %i bpp modes\n", req_bitpp );
	   goto abort_error;
        }
        if( alt_request_bitpp )
        {
	    if(request_bitpp != alt_request_bitpp)
	    {
	       request_bitpp = alt_request_bitpp;
	       continue;
	    }
	    GenPrintf( EMSG_info,"No %s modes\n", req_errmsg );
	}
        request_bitpp = 8;  // default last attempt
    }
    // requested modes failed, and 8bpp failed
    GenPrintf( EMSG_info,"Draw 8bpp using palette, SDL must convert to %i bpp video modes\n", videoInfo->vfmt->BitsPerPixel );
    request_NULL = 1;
    modeList = SDL_ListModes(NULL, surfaceFlags_fullscreen);
    if(modeList == NULL)
    {
        // should not happen with fullscreen modes
        GenPrintf( EMSG_error, "No usable fullscreen video modes.\n");
        goto abort_error;
    }

found_modes:
    // Have some requested video modes in modeList
    vid.bitpp = request_bitpp;

    numVidModes=0;
    firstEntry = -1;
    // Prepare Mode List
    while(modeList[numVidModes])
    {
        if( verbose )
        {
	    // list the modes
	    GenPrintf( EMSG_ver, "%s %ix%i",
		     (((numVidModes&0x03)==0)?(numVidModes)?"\nModes ":"Modes ":""),
		     modeList[numVidModes]->w, modeList[numVidModes]->h );
	}
        if( firstEntry < 0 )
        {
	    if(modeList[numVidModes]->w <= MAXVIDWIDTH &&
	       modeList[numVidModes]->h <= MAXVIDHEIGHT)
	    {
	        firstEntry = numVidModes;
	    }
	}
        numVidModes++;
    }
    // Mode List has been prepared

    if( verbose )
       GenPrintf( EMSG_ver, "\nFound %d Video Modes at %i bpp\n", numVidModes, vid.bitpp);

    allow_fullscreen = true;
    mode_fullscreen = select_fullscreen;  // initial startup

// [WDJ] To be safe, make it conditional
#ifdef MAC_SDL
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
       if (HWD.pfnGetRenderVersion() != DOOMLEGACY_COMPONENT_VERSION )
       {
           I_Error ("The version of the renderer doesn't match the version of the executable\nBe sure you have installed DoomLegacy properly.\n");
       }

       // keep voodoo fixes from messing with the initial window size
       if( detect_voodoo() )
       {
	   vid.width = 640; // hack to make voodoo cards work in 640x480
	   vid.height = 480;
       }
       vid.fullscreen = mode_fullscreen;
       vid.widthbytes = vid.width * vid.bytepp;

       if( verbose>1 )
	  GenPrintf( EMSG_ver, "OglSdlSurface(%i,%i,%i)\n", vid.width, vid.height, mode_fullscreen);
       if(!OglSdlSurface(vid.width, vid.height, mode_fullscreen))
           rendermode = render_soft;
    }
    else
    {
        rendermode = render_soft;
    }

    if(render_soft == rendermode)
    {
        modenum_t initialmode = VID_GetModeForSize(vid.width, vid.height,
		   (select_fullscreen ? MODE_fullscreen: MODE_window));
        VID_SetMode( initialmode );
        if(vidSurface == NULL)
        {
            GenPrintf( EMSG_error,"Could not set vidmode\n");
            goto abort_error;
        }
    }
    I_StartupMouse( false );

    graphics_state = VGS_fullactive;

#if defined(MAC_SDL) && defined( DEBUG_MAC )
    SDL_Delay( 4 * 1000 );  // [WDJ] DEBUG: to see if errors are due to startup or activity
#endif
    return;  // have video mode

abort_error:
    // cannot return without a display screen
    I_Error("RequestFullGraphics Abort\n");
}


void I_ShutdownGraphics()
{
    // was graphics initialized anyway?
    if( graphics_state <= VGS_shutdown )
        return;

    graphics_state = VGS_shutdown;  // to catch some repeats due to errors

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
    graphics_state = VGS_off;

#if defined(MAC_SDL) && defined( DEBUG_MAC )
    GenPrintf( EMSG_info,"SDL_Quit()\n");  // [WDJ] DEBUG:
#endif
    SDL_Quit();
}
