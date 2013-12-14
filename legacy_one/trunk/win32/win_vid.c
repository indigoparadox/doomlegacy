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
//
// $Log: win_vid.c,v $
// Revision 1.22  2003/06/05 20:40:17  hurdler
// accept -glide command line switch
//
// Revision 1.21  2002/09/21 11:10:28  hurdler
//
// Revision 1.20  2001/08/06 23:57:11  stroggonmeth
// Removed portal code, improved 3D floors in hardware mode.
//
// Revision 1.19  2001/03/30 17:12:52  bpereira
// Revision 1.18  2001/02/10 12:27:15  bpereira
//
// Revision 1.17  2001/01/25 22:15:45  bpereira
// added heretic support
//
// Revision 1.16  2001/01/06 22:21:08  judgecutor
// Added NoDirectInput mouse input
//
// Revision 1.15  2001/01/05 18:19:48  hurdler
// add renderer version checking
//
// Revision 1.14  2000/11/04 16:23:45  bpereira
// Revision 1.13  2000/11/02 19:49:40  bpereira
// Revision 1.12  2000/10/21 08:43:32  bpereira
//
// Revision 1.11  2000/10/04 16:25:57  hurdler
// Change all those "3dfx names" to more appropriate names
//
// Revision 1.10  2000/10/01 10:18:23  bpereira
// Revision 1.9  2000/09/28 20:57:22  bpereira
//
// Revision 1.8  2000/09/25 19:30:17  hurdler
// Enable Direct3D support as OpenGL
//
// Revision 1.7  2000/09/01 19:34:38  bpereira
// Revision 1.6  2000/08/10 19:58:05  bpereira
//
// Revision 1.5  2000/08/10 17:04:22  hurdler
// add ticrate to hardware mode
//
// Revision 1.4  2000/08/10 14:19:56  hurdler
// add waitvbl
//
// Revision 1.3  2000/02/27 00:42:12  hurdler
// Revision 1.2  2000/02/26 00:28:42  hurdler
// Mostly bug fix (see borislog.txt 23-2-2000, 24-2-2000)
//
//
// DESCRIPTION:
//      win32 video driver for Doom Legacy
//
//-----------------------------------------------------------------------------

// Because of WINVER redefine, doomtype.h (via doomincl.h) is before any
// other include that might define WINVER
#include "../doomincl.h"

#include <stdlib.h>
#include <stdarg.h>

#include "../i_system.h"
#include "../m_argv.h"
#include "../v_video.h"
#include "../st_stuff.h"
#include "../i_video.h"
#include "../z_zone.h"
#include "fabdxlib.h"       //wow! I can make use of my win32 test stuff!!

#include "win_main.h"
#include "win_vid.h"

#ifdef HWRENDER
#include "win_dll.h"                //loading the Glide Render DLL
#include "../hardware/hw_drv.h"     //calling Driver Init & Shutdown
#include "../hardware/hw_main.h"    //calling HWR module Init & Shutdown
#endif

// -------
// Globals
// -------

static  BOOL        DIB_mode;  // means we are using DIB instead of DirectDraw surfaces
static  BITMAPINFO* bmi_main = NULL;
static  HDC         hDC_main = NULL;

static  BOOL  req_win;
static  byte  request_bitpp = 0;  // to select modes
static  byte  highcolor = 0;


// -----------------
// Video modes stuff
// -----------------

#define MAX_EXTRA_MODES         30
static  vmode_t     extra_modes[MAX_EXTRA_MODES] = {{NULL, NULL}};
static  char        names[MAX_EXTRA_MODES][10];

//static  int     totalvidmem;

int     numvidmodes;   //total number of DirectDraw display modes
vmode_t *pvidmodes;    //start of videomodes list.
vmode_t *pcurrentmode; // the current active videomode.

static int VID_SetWindowedDisplayMode (viddef_t *lvid, vmode_t *pcurrentmode);

// this holds description of the startup video mode,
// the resolution is 320x200, windowed on the desktop
#define NUMSPECIALMODES  2
vmode_t specialmodes[NUMSPECIALMODES] = {
        {   // 0 mode, HIDDEN
            & specialmodes[1],
            "Initial",
            INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
            INITIAL_WINDOW_WIDTH, 1,     // rowbytes, bytes per pixel
            1, 2,  // windowed, numpages
            NULL,
            VID_SetWindowedDisplayMode,
            0          // misc
        },
        {
            NULL,
            "320x200W", //faB: W to make sure it's the windowed mode
            320, 200,   //(200.0/320.0)*(320.0/240.0),
            320, 1,     // rowbytes, bytes per pixel
            1, 2,       // windowed (TRUE), numpages
            NULL,
            VID_SetWindowedDisplayMode,
            0          // misc
        }
};


// ------
// Protos
// ------
static  void VID_Command_NumModes_f (void);
static  void VID_Command_ModeInfo_f (void);
static  void VID_Command_ModeList_f (void);
static  void VID_Command_Mode_f     (void);
static  int VID_SetDirectDrawMode (viddef_t *lvid, vmode_t *pcurrentmode);
static  int VID_SetWindowedDisplayMode (viddef_t *lvid, vmode_t *pcurrentmode);
        vmode_t *VID_GetModePtr (int modenum);
static  void VID_Init (void);
static  void VID_GetModes (void);

// judgecutor:
extern void I_RestartSysMouse();


// -----------------
// I_StartupGraphics
// Initialize video mode, setup dynamic screen size variables,
// and allocate screens.
// -----------------
// May be called more than once, to change modes and switches
void I_StartupGraphics(void)
{
    if( ! graphics_started )
    {
        VID_Init();

        COM_AddCommand ("vid_nummodes", VID_Command_NumModes_f);
        COM_AddCommand ("vid_modeinfo", VID_Command_ModeInfo_f);
        COM_AddCommand ("vid_modelist", VID_Command_ModeList_f);
        COM_AddCommand ("vid_mode", VID_Command_Mode_f);

        //added:03-01-98: register exit code for graphics
        I_AddExitFunc (I_ShutdownGraphics);
    }

    // 0 for 256 color, else use highcolor modes
    highcolor = (req_drawmode == REQ_highcolor);

#ifdef HWRENDER   
    if (M_CheckParm ("-3dfx"))
        rendermode = render_glide;
    else if (M_CheckParm ("-glide"))
        rendermode = render_glide;
    else if (M_CheckParm ("-opengl"))
        rendermode = render_opengl;
    else if (M_CheckParm ("-minigl")) // MiniGL is considered like ...
        rendermode = render_opengl;   // ... OpenGL in the main code
    else if (M_CheckParm ("-d3d"))
        rendermode = render_d3d;
    else
#endif
        rendermode = render_soft;

    // if '-win' is specified on the command line, do not add DirectDraw modes
    req_win = M_CheckParm ("-win");
//    if ( req_win )
//        rendermode  = render_soft;

    VID_GetModes();
    // set the startup screen in a window
    VID_SetMode (0);

    graphics_started = TRUE;
}


// ------------------
// I_ShutdownGraphics
// Close the screen, restore previous video mode.
// ------------------
void I_ShutdownGraphics (void)
{
    if (!graphics_started)
        return;

    CONS_Printf ("I_ShutdownGraphics()\n");

    // release windowed startup stuff
    if (hDC_main) {
        ReleaseDC (hWnd_main, hDC_main);
        hDC_main = NULL;
    }
    if (bmi_main) {
        GlobalFree (bmi_main);
        bmi_main = NULL;
    }

#ifdef HWRENDER
    if ( rendermode != render_soft )
    {
        // Hurdler: swap des deux lignes comme ça on close
        //          l'environnement OpenGL/glide après avoir 
        //          vidé la cache ce qui est bcp plus propre
        HWR_Shutdown ();      //free stuff from the hardware renderer
        HWD.pfnShutdown ();   //close 3d card display
        Shutdown3DDriver ();  //free the driver DLL
    }
#endif

    // free the last video mode screen buffers
    if (vid.buffer) {
        GlobalFree (vid.buffer);
        vid.buffer = NULL;
        vid.display = NULL;
    }

    if ( rendermode == render_soft )
    {
        //HWD.pfnShutdown ();
        //ShutdownSoftDriver ();
        FDX_CloseDirectDraw ();
    }

    graphics_started = FALSE;
}


// ------------
// I_StartFrame
// ------------
void I_StartFrame (void)
{
    //faB: no use
}


// --------------
// I_UpdateNoBlit
// --------------
void I_UpdateNoBlit (void)
{
    // what is this?
}


// --------------
// I_FinishUpdate
// --------------
void I_FinishUpdate (void)
{
    //RECT        Rect;

    //
    // If page flip involves changing vid.display, then must change screens[0] too
    if ( DIB_mode )
    {
        // paranoia
        if ( !hDC_main || !bmi_main || !vid.buffer )
            return;
        // main game loop, still in a window (-win parm)
        SetDIBitsToDevice (hDC_main,
                           0, 0, 320, 200,
                           0, 0, 0, 200,
                           vid.display, bmi_main, DIB_RGB_COLORS);
    }
#ifdef HWRENDER
    else
    if (rendermode != render_soft) {
        HWD.pfnFinishUpdate ( cv_vidwait.value );
    }
#endif
    else
    {
        // DIRECT DRAW
        // copy virtual screen to real screen
        // 26-12-99 BP: can fail when not active (alt-tab)
        if(FDX_LockScreen())
        {
            //faB: TODO: use directX blit here!!? a blit might use hardware with access
            //     to main memory on recent hardware, and software blit of directX may be
            //  optimized for p2 or mmx??
            VID_BlitLinearScreen (vid.display, ScreenPtr,
                                  vid.widthbytes, vid.height, // copy area
                                  vid.ybytes, ScreenPitch ); // scanline inc

            FDX_UnlockScreen();

            // swap screens
            FDX_ScreenFlip(cv_vidwait.value);
        }
    }
}



// for Win32 version
static byte  WndNumpages;


//
// This is meant to be called only by CONS_Printf() while game startup
//
// printf to loading screen
void I_LoadingScreen ( const char * msg )
{
    //PAINTSTRUCT ps;
    RECT        rect;
    //HDC         hdc;

    // paranoia
    if ( !hDC_main || !bmi_main || !vid.buffer || !hWnd_main )
        return;

    //hdc = BeginPaint (hWnd_main, &ps);
    GetClientRect (hWnd_main, &rect);

    SetDIBitsToDevice (hDC_main,
                       0, 0, 320, 200,
                       0, 0, 0, 200,
                       vid.display, bmi_main, DIB_RGB_COLORS);

    if ( msg )
    {
        if ( rect.bottom - rect.top > 32 )
            rect.top = rect.bottom - 32;        // put msg on bottom of window
        SetBkMode ( hDC_main, TRANSPARENT );
        SetTextColor ( hDC_main, RGB(0xff,0xff,0xff) );
        DrawText (hDC_main, msg, -1, &rect,
                  DT_WORDBREAK | DT_CENTER ); //| DT_SINGLELINE | DT_VCENTER);
    }
    //EndPaint (hWnd_main, &ps);
}


// ------------
// I_ReadScreen
// ------------
void I_ReadScreen (byte* scr)
{
#ifdef HWRENDER
    // DEBUGGING
    if (rendermode != render_soft)
        I_SoftError ("I_ReadScreen: called while in non-software mode");
#endif
    CopyMemory (scr, vid.display, vid.screen_size);
}


// ------------
// I_SetPalette
// ------------
void I_SetPalette (RGBA_t *palette)
{
    int   i;

    if ( DIB_mode )
    {
        // set palette in RGBQUAD format, NOT THE SAME ORDER as PALETTEENTRY, grmpf!
        RGBQUAD*    pColors;
        pColors = (RGBQUAD*) ((char*)bmi_main + bmi_main->bmiHeader.biSize);
        ZeroMemory (pColors, sizeof(RGBQUAD)*256);
        for (i=0; i<256; i++, pColors++,palette++)
        {
            pColors->rgbRed = palette->s.red;
            pColors->rgbGreen = palette->s.green;
            pColors->rgbBlue = palette->s.blue;
        }
    }
    else
    if( rendermode == render_soft )
    {
        PALETTEENTRY    mainpal[256];

        // this clears the 'flag' for each color in palette
        ZeroMemory (mainpal, sizeof(mainpal));

        // set palette in PALETTEENTRY format
        for (i=0; i<256; i++,palette++)
        {
            mainpal[i].peRed = palette->s.red;
            mainpal[i].peGreen = palette->s.green;
            mainpal[i].peBlue = palette->s.blue;
        }
#if ( defined(DEBUG_WINDOWED) && defined(WIN32) )
        // Palette fix during debug, otherwise black text on black background
        if( palette[6].s.red < 96 )
	    mainpal[6].peRed = 96;  // at least get red text on black
        if( palette[7].s.green < 96 )
	    mainpal[i].peGreen = 96;  // at least get green text on black
#endif
        FDX_SetDDPalette (mainpal);         // set DirectDraw palette
    }
}


// for debuging
void IO_Color( byte color, byte r, byte g, byte b )
{
/*
outportb( 0x03c8 , color );                // registre couleur
outportb( 0x03c9 , (r>>2) & 0x3f );       // R
outportb( 0x03c9 , (g>>2) & 0x3f );       // G
outportb( 0x03c9 , (b>>2) & 0x3f );       // B
    */
}


//
// return number of video modes in pvidmodes list
//
int VID_NumModes(void)
{
    return numvidmodes - NUMSPECIALMODES;   //faB: dont accept the windowed mode 0
}


// return a video mode number from the dimensions
// returns any available video mode if the mode was not found
int VID_GetModeForSize( unsigned int w, unsigned int h)
{
    vmode_t *pv;
    int     modenum;

#if NUMSPECIALMODES > 1
#error "fix this : pv must point the first fullscreen mode in vidmodes list"
#endif
    // skip the 1st special mode so that it finds only fullscreen modes
    pv = pvidmodes->pnext;
    for (modenum=1; pv!=NULL; pv=pv->pnext,modenum++ )
    {
        if( pv->width ==w &&
            pv->height==h )
            return modenum;
    }

    // if not found, return the first mode avaialable,
    // preferably a full screen mode (all modes after the 'specialmodes')
    if (numvidmodes > NUMSPECIALMODES)
        return NUMSPECIALMODES;         // use first full screen mode

    return 0;   // no fullscreen mode, use windowed mode
}


//
// Enumerate DirectDraw modes available
//
static  int     nummodes=0;

static BOOL VID_DDModes_callback (int width, int height, int bpp)
{
    CONS_Printf ("mode %d x %d x %d bpp\n", width, height, bpp);

    // skip all unwanted modes
    if (highcolor && (bpp != 15))
        goto skip;
    if (!highcolor && (bpp != 8))
        goto skip;

    if ((bpp > 16) ||
        (width > MAXVIDWIDTH) ||
        (height > MAXVIDHEIGHT))
    {
        goto skip;
    }

    // check if we have space for this mode
    if (nummodes>=MAX_EXTRA_MODES)
    {
        CONS_Printf ("mode skipped (too many)\n");
        return FALSE;
    }

        //DEBUG: test without 320x200 standard mode
        //if (width<640 || height<400)
        //    goto skip;

    // store mode info
    extra_modes[nummodes].pnext = &extra_modes[nummodes+1];
    if (width > 999)
    {
        if (height > 999)
        {
            sprintf (&names[nummodes][0], "%4dx%4d", width, height);
            names[nummodes][9] = 0;
        }
        else
        {
            sprintf (&names[nummodes][0], "%4dx%3d", width, height);
            names[nummodes][8] = 0;
        }
    }
    else
    {
        if (height > 999)
        {
            sprintf (&names[nummodes][0], "%3dx%4d", width, height);
            names[nummodes][8] = 0;
        }
        else
        {
            sprintf (&names[nummodes][0], "%3dx%3d", width, height);
            names[nummodes][7] = 0;
        }
    }

    extra_modes[nummodes].name = &names[nummodes][0];
    extra_modes[nummodes].width = width;
    extra_modes[nummodes].height = height;

    // exactly, the current FinishUdpate() gets the rowbytes itself after locking the video buffer
    // so for now we put anything here
    extra_modes[nummodes].rowbytes = width;
    extra_modes[nummodes].windowed = false;
    extra_modes[nummodes].misc = 0;         // unused
    extra_modes[nummodes].pextradata = NULL;
    extra_modes[nummodes].setmode = VID_SetDirectDrawMode;

    extra_modes[nummodes].numpages = 2;     // double-buffer (but this value is not used)

    extra_modes[nummodes].bytesperpixel = (bpp+1)>>3;

    nummodes++;
skip:
    return TRUE;  // continue
}


//
// Collect info about DirectDraw display modes we use
//
void VID_GetExtraModes (void)
{
    nummodes = 0;
    FDX_EnumDisplayModes (VID_DDModes_callback);

    // add the extra modes (non 320x200) at the start of the mode list (if there are any)
    if (nummodes)
    {
        extra_modes[nummodes-1].pnext = NULL;
        pvidmodes = &extra_modes[0];
        numvidmodes += nummodes;
    }
}


// ---------------
// WindowMode_Init
// Add windowed modes to the start of the list,
// mode 0 is used for windowed console startup (works on all computers with no DirectX)
// ---------------
static void WindowMode_Init(void)
{
    specialmodes[NUMSPECIALMODES-1].pnext = pvidmodes;
    pvidmodes = &specialmodes[0];
    numvidmodes += NUMSPECIALMODES;
}



// *************************************************************************************
// VID_Init
// Initialize Video modes subsystem
// *************************************************************************************
// Called from I_StartupGraphics
void VID_Init (void)
{
    // initialize the appropriate display device
}

// May be called more than once, to change modes and switches
void VID_GetModes (void)
{
    vmode_t*    pv;
    int         iMode;
    char * req_errmsg = NULL;
    byte  alt_request_bitpp = 0;

//  unsigned int screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
//  unsigned int screen_height = GetSystemMetrics(SM_CYFULLSCREEN);

    switch(req_drawmode)
    {
     case REQ_native:
       vid.bitpp = GetDeviceCaps( GetDC( hWnd_main ), BITSPIXEL );
       vid.bytepp = (vid.bitpp + 7) >> 3;
       if( V_CanDraw( vid.bitpp )) {
	   request_bitpp = vid.bitpp;
       }else{
	   // Use 8 bit and do the palette lookup.
	   if( verbose )
	       GenPrintf(EMSG_ver, "Native %i bpp rejected\n", vid.bitpp );
	   request_bitpp = 8;
       }
       break;
     case REQ_specific:
       request_bitpp = req_bitpp;
       break;
     case REQ_highcolor:
       req_errmsg = "highcolor";
       request_bitpp = 15;
       alt_request_bitpp = 16;
//       if( vid.bitpp == 16 )  request_bitpp = 16;  // native preference
       break;
     case REQ_truecolor:
       req_errmsg = "truecolor";
       request_bitpp = 24;
       alt_request_bitpp = 32;
//       if( vid.bitpp == 32 )  request_bitpp = 32;  // native preference
       break;
     default:
       request_bitpp = 8;  // default native
       break;
    }

    //setup the videmodes list,
    // note that mode 0 must always be VGA mode 0x13
    pvidmodes = NULL;
    pcurrentmode = NULL;
    numvidmodes = 0;

    vid.buffer = NULL;
    vid.display = NULL;

    // we startup in windowed mode using DIB bitmap
    // we will use DirectDraw when switching fullScreen and entering main game loop
    DIB_mode = TRUE;
    vid.fullscreen = FALSE;

    // initialize the appropriate display device
#ifdef HWRENDER
    if ( rendermode != render_soft )
    {
        char* drvname;

        switch (rendermode)
        {
            case render_glide: drvname = "r_glide.dll"; break;
            case render_opengl: 
            // Here is the only difference between OpenGL and MiniGL in the main code
                if (M_CheckParm ("-opengl"))
                    drvname = "r_opengl.dll";
                else
                    drvname = "r_minigl.dll";
                break;
            case render_d3d:   drvname = "r_d3d.dll"; break;
            default:
	       I_Error ("Unknown hardware render mode");
	       return;
        }

        // load the DLL
        if ( Init3DDriver (drvname) )
        {
            int hwdversion = HWD.pfnGetRenderVersion();
            if ( hwdversion != VERSION)
            {
                if (rendermode != render_glide)
                {
                    I_Error ("The version of the renderer (v%d.%d) doesn't match the version of the executable (v%d.%d)\n"
                             "Be sure you have installed Doom Legacy properly.\n"
                             "Eventually verify the launcher settings.\n", 
                             hwdversion/100, hwdversion%100,
                             VERSION/100, VERSION%100);
                }
                else
                {   
                    CONS_Printf("WARNING: This r_glide version is not supported, use it at your own risks.\n");
                }
            }
            // perform initialisations
            HWD.pfnInit ((I_Error_t)I_Error);
            // get available display modes for the device
            HWD.pfnGetModeList (&pvidmodes, &numvidmodes);
        }
        else
        {
            switch (rendermode) {
                case render_glide:
                    I_Error ("Error initializing Glide\n");
                    break;
                case render_opengl:
                    I_Error ("Error initializing OpenGL\n");
                    break;
                case render_d3d:
                    I_Error ("Error initializing Direct3D\n");
                    break;
                default: break;
            }
            rendermode = render_soft;
        }
    }
#endif
    if (rendermode == render_soft && !req_win )
    {
        FDX_create_main_instance();

        // try the requested bpp, then alt, then 8bpp
        for(;;)
        {
	    // get available display modes for the device
	    VID_GetExtraModes ();
	    if(numvidmodes) goto found_modes;

	    if( request_bitpp == 8 )  break;
	    if(req_drawmode == REQ_specific)
	    {
	        GenPrintf(EMSG_error, "No %i bpp modes\n", req_bitpp );
	        goto abort_error;
	    }
	    if( alt_request_bitpp )
	    {
	        if(request_bitpp != alt_request_bitpp)
	        {
		    request_bitpp = alt_request_bitpp;
		    continue;
		}
	        GenPrintf(EMSG_error, "No %s modes\n", req_errmsg );
	        // win32 had -highcolor as binding, so do not change that behavior
	        goto abort_error;
	    }
	    request_bitpp = 8;  // default last attempt
	}
    }
    // assumes there is always a default 8bpp mode

    // the game boots in 320x200 standard VGA, but
    // we need a highcolor mode to run the game in highcolor
    if (request_bitpp>8 && numvidmodes==0)
        I_Error ("No highcolor/truecolor VESA2 video mode found, cannot run in highcolor/truecolor.\n");

found_modes:
    vid.bitpp = request_bitpp;
    vid.bytepp = (request_bitpp + 7) >> 3;

    // add windowed mode at the start of the list, very important!
    WindowMode_Init();

    if (numvidmodes==0)
        I_Error ("No display modes available.\n");

    //DEBUG
    for (iMode=0,pv=pvidmodes; pv; pv=pv->pnext,iMode++)
    {
        CONS_Printf ("%#02d: %dx%dx%dbpp (desc: '%s')\n",iMode,
                     pv->width,pv->height,pv->bytesperpixel,pv->name);
    }
    return;

abort_error:
    // cannot return without a display screen
    I_Error("StartupGraphics/VID_Init Abort\n");
}


// --------------------------
// VID_SetWindowedDisplayMode
// Display the startup 320x200 console screen into a window on the desktop,
// switching to fullscreen display only when we will enter the main game loop.
// - we can display error message boxes for startup errors
// - we can set the last used resolution only once, when entering the main game loop
// --------------------------
static int VID_SetWindowedDisplayMode (viddef_t *lvid, vmode_t *pcurrentmode)
{
    int     screen_width, screen_height;
    int     window_width, window_height;
    //RECT    Rect;

#ifdef DEBUG
    CONS_Printf("VID_SetWindowedDisplayMode()\n");
#endif

    WndNumpages = 1;      // not used
    lvid->direct = NULL;  // DOS remains
    lvid->buffer = NULL;

    // allocate screens
    if (!VID_FreeAndAllocVidbuffer (lvid))
        return -1;

    // lvid->buffer should be NULL here!

    if ((bmi_main = (void*)GlobalAlloc (GPTR, sizeof(BITMAPINFO) + (sizeof(RGBQUAD)*256)))==NULL)
        I_Error ("VID_SWDM(): No mem");

    // setup a BITMAPINFO to allow copying our video buffer to the desktop,
    // with color conversion as needed
    bmi_main->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi_main->bmiHeader.biWidth = lvid->width;
    bmi_main->bmiHeader.biHeight= -(lvid->height);
    bmi_main->bmiHeader.biPlanes = 1;
    bmi_main->bmiHeader.biBitCount = 8;
    bmi_main->bmiHeader.biCompression = BI_RGB;

    // center window on the desktop
    screen_width = GetSystemMetrics(SM_CXFULLSCREEN);
    screen_height = GetSystemMetrics(SM_CYFULLSCREEN);

    window_width = lvid->width;
    window_width += GetSystemMetrics(SM_CXFIXEDFRAME) * 2;

    window_height = lvid->height;
    window_height += GetSystemMetrics(SM_CYCAPTION);
    window_height += GetSystemMetrics(SM_CYFIXEDFRAME) * 2;

    if( devparm )
        MoveWindow (hWnd_main, (screen_width - window_width)   , (screen_height - window_height)   , window_width, window_height, TRUE);
    else
        MoveWindow (hWnd_main, (screen_width - window_width)>>1, (screen_height - window_height)>>1, window_width, window_height, TRUE);

    SetFocus(hWnd_main);
    ShowWindow(hWnd_main, SW_SHOW);

    hDC_main = GetDC(hWnd_main);
    if( !hDC_main )
        I_Error ("VID_SWDM(): GetDC FAILED");
    //SetStretchBltMode (hDC_main, COLORONCOLOR);

    return 1;
}


// ========================================================================
// Returns a vmode_t from the video modes list, given a video mode number.
// ========================================================================
vmode_t *VID_GetModePtr (int modenum)
{
    vmode_t *pv;

    pv = pvidmodes;
    if (!pv)
        I_Error ("VID_error : No video mode found\n");

    while (modenum--)
    {
        pv = pv->pnext;
        if (!pv)
            I_Error ("VID_error : Mode not available\n");
    }
    return pv;
}


//
// return the name of a video mode
//
char* VID_GetModeName (int modenum)
{
    return (VID_GetModePtr(modenum))->name;
}


// ========================================================================
// Sets a video mode
// ========================================================================
int VID_SetMode (int modenum)  //, unsigned char *palette)
{
    int     stat;
    vmode_t *pnewmode, *poldmode;

    CONS_Printf("VID_SetMode(%d)\n",modenum);

    //faB: if mode 0 (windowed) we must not be fullscreen already,
    //     if other mode, check it is not mode 0 and existing
    if ((modenum != 0) || (vid.fullscreen))
    {
        if ((modenum > numvidmodes) || (modenum < NUMSPECIALMODES))
        {
            if (pcurrentmode == NULL)
                modenum = 0;    // revert to the default base vid mode
            else
            {
                //nomodecheck = TRUE;
                I_Error ("Unknown video mode: %d\n", modenum);
                //nomodecheck = FALSE;
                return 0;
            }
        }
    }

    pnewmode = VID_GetModePtr (modenum);

    // dont switch to the same display mode
    if (pnewmode == pcurrentmode)
        return 1;

    // initialize the new mode
    poldmode = pcurrentmode;
    pcurrentmode = pnewmode;

    // initialize vidbuffer size for setmode
    vid.width  = pcurrentmode->width;
    vid.height = pcurrentmode->height;
    //vid.aspect = pcurrentmode->aspect;                // aspect ratio might be needed later for 3dfx version..
    vid.direct_rowbytes = pcurrentmode->rowbytes;
    vid.bytepp = pcurrentmode->bytesperpixel;
    vid.bitpp = (vid.bytepp==1)? 8:15;
    vid.drawmode = (vid.bytepp==1)? DRAW8PAL:DRAW15;
#ifdef HWRENDER
    //hurdler: 15/10/99: added
    if (modenum) { // if not 320x200 windowed mode
        // it's actually a hack
        if ( (rendermode == render_opengl) || (rendermode == render_d3d) ) {
            // don't accept depth < 16 for OpenGL mode (too much ugly)
            if (cv_scr_depth.value<16)
                CV_SetValue (&cv_scr_depth,  16);
            vid.bitpp = cv_scr_depth.value;
            vid.bytepp = cv_scr_depth.value/8;
            vid.fullscreen = cv_fullscreen.value;
            pcurrentmode->bytesperpixel = vid.bytepp;
            pcurrentmode->windowed = ! vid.fullscreen;
        }
    }
#endif
    vid.widthbytes = vid.width * vid.bytepp;
    vid.direct_size = vid.direct_rowbytes * vid.height;

    stat = (*pcurrentmode->setmode) (&vid, pcurrentmode);
      // sets vid.direct, vid.buffer, vid.display, vid.ybytes, vid.screen_size, vid.screen1

    if (stat < 1)
    {
        if (stat == 0)
        {
            // harware could not setup mode
            //if (!VID_SetMode (vid.modenum))
            //        I_Error ("VID_SetMode: couldn't set video mode (hard failure)");
            I_Error ("Couldn't set video mode %d (%dx%d %d bits)\n", modenum, vid.width, vid.height, vid.bitpp);
        }
        else
            if (stat == -1)
            {
                I_Error ("Not enough mem for VID_SetMode\n");

                // not enough memory; just put things back the way they were
                /*pcurrentmode = poldmode;
                vid.width = pcurrentmode->width;
                vid.height = pcurrentmode->height;
                vid.rowbytes = pcurrentmode->rowbytes;
                vid.bytepp  = pcurrentmode->bytesperpixel;
                vid.bitpp = vid.bytepp * 8;
                return 0;*/
            }
    }

    vid.modenum = modenum;

    // tell game engine to recalc all tables and realloc buffers based on
    // new vid values
    vid.recalc = 1;

    if ( modenum < NUMSPECIALMODES )
    {
        // we are in startup windowed mode
        vid.fullscreen = false;
        DIB_mode = TRUE;
    }
    else
    {
        // we switch to fullscreen
        vid.fullscreen = fdx_fullscreen;
        DIB_mode = FALSE;
    }

    // judgecutor:
    I_RestartSysMouse();
    return 1;
}


// ========================================================================
// Free the video buffer of the last video mode,
// allocate a new buffer for the video mode to set.
// ========================================================================
BOOL    VID_FreeAndAllocVidbuffer (viddef_t *lvid)
{
    int  vidbuffersize;

    // Must agree with FinishUpdate, which uses VID_BlitLinearScreen
#if 1 
    // screen size same as video buffer, simple copy
    lvid->ybytes = lvid->direct_rowbytes;
    lvid->screen_size = lvid->direct_size;
#else
    // minimal screen buffer, must copy by line (VID_BlitLinearScreen)
    lvid->ybytes = lvid->widthbytes;
    lvid->screen_size = lvid->ybytes * lvid->height;
#endif
    vidbuffersize = (lvid->screen_size * NUMSCREENS);

    // free allocated buffer for previous video mode
    if (lvid->buffer)
        GlobalFree (lvid->buffer);

    // allocate & clear the new screen buffer
    lvid->buffer = GlobalAlloc (GPTR, vidbuffersize);
    lvid->display = lvid->buffer;  // display = buffer, screen[0]
    if( lvid->buffer == NULL )
    {
        lvid->screen1 = NULL;
        return FALSE;
    }
    lvid->screen1 = lvid->buffer + lvid->screen_size;

#ifdef DEBUG
    CONS_Printf("VID_FreeAndAllocVidbuffer done, vidbuffersize: %x\n",vidbuffersize);
#endif
    return TRUE;
}


// ========================================================================
// Set video mode routine for DirectDraw display modes
// Out: 1 ok,
//      0 hardware could not set mode,
//     -1 no mem
// ========================================================================
static int VID_SetDirectDrawMode (viddef_t *lvid, vmode_t *pcurrentmode)
{

#ifdef DEBUG
    CONS_Printf("VID_SetDirectDrawMode...\n");
#endif

    // DD modes do double-buffer page flipping, but the game engine doesn't need this..
    WndNumpages = 2;

//MessageBox (hWnd_main, "switch full screen","bla",MB_OK|MB_ICONERROR);

    // release ddraw surfaces etc..
    FDX_ReleaseChtuff();

    // clean up any old vid buffer lying around, alloc new if needed
    if (!VID_FreeAndAllocVidbuffer (lvid))
        return -1;                  //no mem

    //added:20-01-98: should clear video mem here

    if (! FDX_InitDDMode(hWnd_main, lvid->width, lvid->height, lvid->bitpp, lvid->fullscreen))
        return 0;               // could not set mode

    // this is NOT used with DirectDraw modes, game engine should never use this directly
    // but rather render to memory bitmap buffer
    lvid->direct = NULL;

    return 1;
}


// ========================================================================
//                     VIDEO MODE CONSOLE COMMANDS
// ========================================================================


//  vid_nummodes
//
static  void VID_Command_NumModes_f (void)
{
    int     nummodes;

    nummodes = VID_NumModes ();
    CONS_Printf ("%d video mode(s) available(s)\n", nummodes);
}


//  vid_modeinfo <modenum>
//
static  void VID_Command_ModeInfo_f (void)
{
    vmode_t     *pv;
    int         modenum;

    if (COM_Argc()!=2)
        modenum = vid.modenum;          // describe the current mode
    else
        modenum = atoi (COM_Argv(1));   //    .. the given mode number

    if (modenum >= VID_NumModes() || modenum<1) //faB: dont accept the windowed mode 0
    {
        CONS_Printf ("No such video mode\n");
        return;
    }

    pv = VID_GetModePtr (modenum);

    CONS_Printf("%s\n", VID_GetModeName (modenum));
    CONS_Printf("width : %d\n"
                "height: %d\n", pv->width, pv->height);
    if (rendermode==render_soft)
    {
        CONS_Printf("bytes per scanline: %d\n"
                    "bytes per pixel: %d\n"
                    "numpages: %d\n",
                    pv->rowbytes,
                    pv->bytesperpixel,
                    pv->numpages);
    }
}


//  vid_modelist
//
static  void VID_Command_ModeList_f (void)
{
    int         i, nummodes;
    char        *pinfo;
    vmode_t     *pv;
    boolean     na;

    na = false;

    nummodes = VID_NumModes ();
    for (i=NUMSPECIALMODES ; i<=nummodes ; i++)
    {
        pv = VID_GetModePtr (i);
        pinfo = VID_GetModeName (i);

        if (pv->bytesperpixel==1)
            CONS_Printf ("%d: %s\n", i, pinfo);
        else
            CONS_Printf ("%d: %s (hicolor)\n", i, pinfo);
    }
}


//  vid_mode <modenum>
//
static  void VID_Command_Mode_f (void)
{
    int         modenum;

    if (COM_Argc()!=2)
    {
        CONS_Printf ("vid_mode <modenum> : set video mode\n");
        return;
    }

    modenum = atoi(COM_Argv(1));

    if (modenum >= VID_NumModes() || modenum<1) //faB: dont accept the windowed mode 0
        CONS_Printf ("No such video mode\n");
    else
        // request vid mode change
        setmodeneeded = modenum+1;
}
