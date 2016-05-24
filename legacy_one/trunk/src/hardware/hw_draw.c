// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1998-2012 by DooM Legacy Team.
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
// $Log: hw_draw.c,v $
// Revision 1.27  2004/07/27 08:19:38  exl
// New fmod, fs functions, bugfix or 2, patrol nodes
//
// Revision 1.26  2004/04/18 20:40:31  hurdler
// go 1.42
//
// Revision 1.25  2004/04/18 12:40:15  hurdler
// Jive's request for saving screenshots
//
// Revision 1.24  2003/10/15 14:09:24  darkwolf95
// Fixed screenshots filename bug
//
// Revision 1.23  2003/08/11 15:12:20  hurdler
// go final + translucent HUD + fix spawn in net game
//
// Revision 1.22  2003/06/11 04:44:11  ssntails
// High-res patch drawer added.
//
// Revision 1.21  2003/06/10 21:48:06  ssntails
// Variable flat size support (32x32 to 2048x2048)
//
// Revision 1.20  2001/08/09 21:35:23  hurdler
// Add translucent 3D water in hw mode
//
// Revision 1.19  2001/05/16 21:21:15  bpereira
// Revision 1.18  2001/04/01 17:35:07  bpereira
// Revision 1.17  2001/02/28 17:50:56  bpereira
// Revision 1.16  2001/02/24 13:35:22  bpereira
//
// Revision 1.15  2001/01/31 17:15:09  hurdler
// Add cv_scalestatusbar in hardware mode
//
// Revision 1.14  2001/01/25 18:56:27  bpereira
// Revision 1.13  2000/11/02 19:49:39  bpereira
// Revision 1.12  2000/10/04 16:21:57  hurdler
//
// Revision 1.11  2000/09/14 10:42:47  hurdler
// Fix compiling problem under win32
//
// Revision 1.10  2000/09/10 10:48:13  metzgermeister
// Revision 1.9  2000/08/31 14:30:57  bpereira
// Revision 1.8  2000/08/11 19:11:57  metzgermeister
//
// Revision 1.7  2000/04/27 17:48:47  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.6  2000/04/24 15:22:47  hurdler
// Support colormap for text
//
// Revision 1.5  2000/04/23 00:30:47  hurdler
// fix a small bug in skin color
//
// Revision 1.4  2000/04/22 21:08:23  hurdler
//
// Revision 1.3  2000/04/14 16:34:26  hurdler
// some nice changes for coronas
//
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      miscellaneous drawing (mainly 2d)
//
//-----------------------------------------------------------------------------

#include "../doomincl.h"

#include "hw_glob.h"
#include "hw_drv.h"

#include "../m_misc.h"
  //FIL_WriteFile()
#include "../m_swap.h"
#include "../r_draw.h"
  //viewborderlump
#include "../r_main.h"
#include "../w_wad.h"
#include "../z_zone.h"
#include "../v_video.h"

#include <unistd.h>
#include <fcntl.h>
#include "../i_video.h"
  // for rendermode != render_glide


#ifdef WIN32
#pragma pack(1)
#endif
typedef struct {  // sizeof() = 18
  byte  id_field_length;
  byte  color_map_type;
  byte  image_type;
  byte  dummy[5];
/*int16_t c_map_origin;
  int16_t c_map_length;
  char  c_map_size;*/
  int16_t x_origin;
  int16_t y_origin;
  uint16_t width;
  uint16_t height;
  byte  image_pix_size;
  byte  image_descriptor;
} TGAHeader_t;
// TGAHeader_t * TGAHeaderp;
#ifdef WIN32
#pragma pack()
#endif
typedef unsigned char GLRGB[3];
void saveTGA(char *file_name, int width, int height, GLRGB *buffer);

#define BLENDMODE PF_Translucent

//
// -----------------+
// HWR_DrawPatch    : Draw a 'tile' graphic
// Notes            : x,y : positions relative to the original Doom resolution
//                  : textes(console+score) + menus + status bar
// -----------------+
void HWR_DrawPatch (MipPatch_t* gpatch, int x, int y, int option)
{
    vxtx3d_t      v[4];

//  3--2
//  | /|
//  |/ |
//  0--1
    float stx, sty, pdupx, pdupy;

    // make patch ready in hardware cache
    HWR_GetPatch (gpatch);

    // V_SetupDraw now has fdupx, fdupy derived from V_SCALEPATCH.
    pdupx = drawinfo.fdupx * 2.0;
    pdupy = drawinfo.fdupy * 2.0;

    // V_SetupDraw now has fdupx0, fdupy0 derived from V_SCALESTART.
    stx = x * drawinfo.fdupx0 * 2.0;
    sty = y * drawinfo.fdupy0 * 2.0;

    v[0].x = v[3].x = (stx - (gpatch->leftoffset*pdupx))/vid.width - 1;
    v[2].x = v[1].x = (stx + ((gpatch->width - gpatch->leftoffset)*pdupx))/vid.width - 1;
    v[0].y = v[1].y = 1-(sty - (gpatch->topoffset*pdupy))/vid.height;
    v[2].y = v[3].y = 1-(sty + ((gpatch->height - gpatch->topoffset)*pdupy))/vid.height;

    v[0].z = v[1].z = v[2].z = v[3].z = 1.0f;

    v[0].sow = v[3].sow = 0.0f;
    v[2].sow = v[1].sow = gpatch->max_s;
    v[0].tow = v[1].tow = 0.0f;
    v[2].tow = v[3].tow = gpatch->max_t;

    // clip it since it is used for bunny scroll in doom I
    if (option & V_TRANSLUCENTPATCH)
    {
        FSurfaceInfo_t Surf;
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = 0xff;
        Surf.FlatColor.s.alpha = cv_grtranslucenthud.value;

        HWD.pfnDrawPolygon( &Surf, v, 4,
            BLENDMODE | PF_Modulated | PF_Clip | PF_NoZClip | PF_NoDepthTest);
    }
    else
    {
        HWD.pfnDrawPolygon( NULL, v, 4,
            BLENDMODE | PF_Clip | PF_NoZClip | PF_NoDepthTest);
    }
}

#if 0
//[WDJ] 2012-02-06 DrawSmallPatch found to be unused

// Draws a patch 2x as small SSNTails 06-10-2003
void HWR_DrawSmallPatch (MipPatch_t* gpatch, int x, int y, int option, byte *colormap)
{
    vxtx3d_t      v[4];

    float stx, sty, pdupx, pdupy;

    // make patch ready in hardware cache
    HWR_GetMappedPatch (gpatch, colormap);

    // V_SetupDraw now has fdupx, fdupy derived from V_SCALEPATCH.
    pdupx = drawinfo.fdupx * 2.0;
    pdupy = drawinfo.fdupy * 2.0;

    // V_SetupDraw now has fdupx0, fdupy0 derived from V_SCALESTART.
    stx = x * drawinfo.fdupx0 * 2.0;
    sty = y * drawinfo.fdupy0 * 2.0;

    v[0].x = v[3].x = (stx - gpatch->leftoffset*pdupx)/vid.width - 1;
    v[2].x = v[1].x = (stx + (gpatch->width - gpatch->leftoffset)*pdupx)/vid.width - 1;
    v[0].y = v[1].y = 1-(sty - gpatch->topoffset*pdupy)/vid.height;
    v[2].y = v[3].y = 1-(sty + (gpatch->height - gpatch->topoffset)*pdupy)/vid.height;

    v[0].z = v[1].z = v[2].z = v[3].z = 1.0f;

    v[0].sow = v[3].sow = 0.0f;
    v[2].sow = v[1].sow = gpatch->max_s;
    v[0].tow = v[1].tow = 0.0f;
    v[2].tow = v[3].tow = gpatch->max_t;

    // clip it since it is used for bunny scroll in doom I
    HWD.pfnDrawPolygon( NULL, v, 4,
        BLENDMODE | PF_Clip | PF_NoZClip | PF_NoDepthTest);
}
#endif

//
// HWR_DrawMappedPatch(): Like HWR_DrawPatch but with translated color
//
void HWR_DrawMappedPatch (MipPatch_t* gpatch, int x, int y, int option, byte *colormap)
{
    vxtx3d_t      v[4];

    float stx, sty, pdupx, pdupy;

    // make patch ready in hardware cache
    HWR_GetMappedPatch (gpatch, colormap);

    // V_SetupDraw now has fdupx, fdupy derived from V_SCALEPATCH.
    pdupx = drawinfo.fdupx * 2.0;
    pdupy = drawinfo.fdupy * 2.0;

    // V_SetupDraw now has fdupx0, fdupy0 derived from V_SCALESTART.
    stx = x * drawinfo.fdupx0 * 2.0;
    sty = y * drawinfo.fdupy0 * 2.0;
    
    v[0].x = v[3].x = (stx - gpatch->leftoffset*pdupx)/vid.width - 1;
    v[2].x = v[1].x = (stx + (gpatch->width - gpatch->leftoffset)*pdupx)/vid.width - 1;
    v[0].y = v[1].y = 1-(sty - gpatch->topoffset*pdupy)/vid.height;
    v[2].y = v[3].y = 1-(sty + (gpatch->height - gpatch->topoffset)*pdupy)/vid.height;

    v[0].z = v[1].z = v[2].z = v[3].z = 1.0f;

    v[0].sow = v[3].sow = 0.0f;
    v[2].sow = v[1].sow = gpatch->max_s;
    v[0].tow = v[1].tow = 0.0f;
    v[2].tow = v[3].tow = gpatch->max_t;

    // clip it since it is used for bunny scroll in doom I
    if (option & V_TRANSLUCENTPATCH)
    {
        FSurfaceInfo_t Surf;
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = 0xff;
        Surf.FlatColor.s.alpha = cv_grtranslucenthud.value;
        HWD.pfnDrawPolygon( &Surf, v, 4,
            BLENDMODE | PF_Modulated | PF_Clip | PF_NoZClip | PF_NoDepthTest);
    }
    else
    {
        HWD.pfnDrawPolygon( NULL, v, 4,
            BLENDMODE | PF_Clip | PF_NoZClip | PF_NoDepthTest);
    }
}

void HWR_DrawPic(int x, int y, int lumpnum)
{
    vxtx3d_t      v[4];
    MipPatch_t  *   mpatch;

    // make pic ready in hardware cache
    mpatch = HWR_GetPic( lumpnum );

//  3--2
//  | /|
//  |/ |
//  0--1

    v[0].x = v[3].x = (float)x * vid.fx_scale2 - 1;
    v[2].x = v[1].x = (float)(x + mpatch->width*vid.fdupx) * vid.fx_scale2 - 1;
    v[0].y = v[1].y = 1 - (float)y * vid.fy_scale2;
    v[2].y = v[3].y = 1 - (float)(y + mpatch->height*vid.fdupy) * vid.fy_scale2;

    v[0].z = v[1].z = v[2].z = v[3].z = 1.0f;

    v[0].sow = v[3].sow =  0;
    v[2].sow = v[1].sow =  mpatch->max_s;
    v[0].tow = v[1].tow =  0;
    v[2].tow = v[3].tow =  mpatch->max_t;


    //Hurdler: Boris, the same comment as above... but maybe for pics
    // it not a problem since they don't have any transparent pixel
    // if I'm right !?
    // But then, the question is: why not 0 instead of PF_Masked ?
    // or maybe PF_Environment ??? (like what I said above)
    // BP: PF_Environment don't change anything ! and 0 is undefined
    if (cv_grtranslucenthud.value != 255)
    {
        FSurfaceInfo_t Surf;
        Surf.FlatColor.s.red = Surf.FlatColor.s.green = Surf.FlatColor.s.blue = 0xff;
        Surf.FlatColor.s.alpha = cv_grtranslucenthud.value;
        HWD.pfnDrawPolygon( &Surf, v, 4,
            BLENDMODE | PF_Modulated | PF_NoDepthTest | PF_Clip | PF_NoZClip);
    }
    else
    {
        HWD.pfnDrawPolygon( NULL, v, 4,
            BLENDMODE | PF_NoDepthTest | PF_Clip | PF_NoZClip);
    }
}

// ==========================================================================
//                                                            V_VIDEO.C STUFF
// ==========================================================================


// --------------------------------------------------------------------------
// Fills a box of pixels using a flat texture as a pattern
// --------------------------------------------------------------------------
// Scaled to (320,200), (0,0) at upper left
void HWR_DrawFlatFill (int x, int y, int w, int h, int flatlumpnum)
{
    vxtx3d_t  v[4];
    double flatsize;
    int flatflag;
    int size;

    size = W_LumpLength(flatlumpnum);

    switch(size)
    {
        case 4194304: // 2048x2048 lump
            flatsize = 2048.0f;
            flatflag = 2047;
            break;
        case 1048576: // 1024x1024 lump
            flatsize = 1024.0f;
            flatflag = 1023;
            break;
        case 262144:// 512x512 lump
            flatsize = 512.0f;
            flatflag = 511;
            break;
        case 65536: // 256x256 lump
            flatsize = 256.0f;
            flatflag = 255;
            break;
        case 16384: // 128x128 lump
            flatsize = 128.0f;
            flatflag = 127;
            break;
        case 1024: // 32x32 lump
            flatsize = 32.0f;
            flatflag = 31;
            break;
        default: // 64x64 lump
            flatsize = 64.0f;
            flatflag = 63;
            break;
    }

//  3--2
//  | /|
//  |/ |
//  0--1

    v[0].x = v[3].x = (x - 160.0f)/160.0f;
    v[2].x = v[1].x = ((x+w) - 160.0f)/160.0f;
    v[0].y = v[1].y = -(y - 100.0f)/100.0f;
    v[2].y = v[3].y = -((y+h) - 100.0f)/100.0f;

    v[0].z = v[1].z = v[2].z = v[3].z = 1.0f;

    // flat is 64x64 lod and texture offsets are [0.0, 1.0]
    v[0].sow = v[3].sow = (x & flatflag)/flatsize;
    v[2].sow = v[1].sow = v[0].sow + w/flatsize;
    v[0].tow = v[1].tow = (y & flatflag)/flatsize;
    v[2].tow = v[3].tow = v[0].tow + h/flatsize;

    HWR_GetFlat (flatlumpnum);

    //Hurdler: Boris, the same comment as above... but maybe for pics
    // it not a problem since they don't have any transparent pixel
    // if I'm right !?
    // BTW, I see we put 0 for PFs, and If I'm right, that
    // means we take the previous PFs as default
    // how can we be sure they are ok?
      // maybe PF_Translucent ??
    HWD.pfnDrawPolygon( NULL, v, 4, PF_NoDepthTest);
}


// --------------------------------------------------------------------------
// Fade down the screen so that the menu drawn on top of it looks brighter
// --------------------------------------------------------------------------
//  3--2
//  | /|
//  |/ |
//  0--1
// Relative to vid.height.
//  color_rgba : rgba color
//  alpha : 1 .. 255
void HWR_FadeScreenMenuBack( uint32_t color_rgba, int alpha, int height )
{
    vxtx3d_t  v[4];
    FSurfaceInfo_t Surf;

    if( height <= 0 )   return;

    // setup some neat-o translucency effect

    // [WDJ] Draws undersized on Matrox on Win32, when it works at all.
    // Many fixes tried, none worked.
    // Exact use of example code may be required, and glGenTextures.
    // May require turning off accel polygon draw.

    v[0].x = v[3].x = -1.0f;
    v[2].x = v[1].x =  1.0f;
    v[0].y = v[1].y = -1.0f;
    v[2].y = v[3].y =  1.0f;
    v[0].z = v[1].z = v[2].z = v[3].z = 1.0f;

    v[0].sow = v[3].sow = 0.0f;
    v[2].sow = v[1].sow = 1.0f;
    v[0].tow = v[1].tow = 1.0f;
    v[2].tow = v[3].tow = 0.0f;

    Surf.FlatColor.rgba = color_rgba;
    // [WDJ] Constant alpha is more reliable.
    Surf.FlatColor.s.alpha = alpha;

    if( height < vid.height )
    {
        // [WDJ] bugginess of this calc when height==vid.height plagued win32.
        float hf2 = ((float)height) * vid.fy_scale2;
        v[0].y = v[1].y = 1.0f - hf2;  // -1.0 .. 1.0
        // [WDJ] This would only give partial alpha, or overflow alpha.
        // Too hard to fix right for such a small visual impact.
//        Surf.FlatColor.s.alpha = alpha * hf;    //calum: varies console alpha
    }
    HWD.pfnDrawPolygon( &Surf, v, 4,
        PF_NoTexture|PF_Modulated|PF_Translucent|PF_NoDepthTest);
}


// ==========================================================================
//                                                             R_DRAW.C STUFF
// ==========================================================================

// ------------------
// HWR_DrawViewBorder
// Fill the space around the view window with a Doom flat texture, draw the
// beveled edges.
// 'clearlines' is useful to clear the heads up messages, when the view
// window is reduced, it doesn't refresh all the view borders.
// ------------------
void HWR_DrawViewBorder (int clearlines)
{
    int         x,y;
    int         top,side;
    int         baseviewwidth,baseviewheight;
    int         basewindowx,basewindowy;
    MipPatch_t * mpatch;

//    if (gr_viewwidth == vid.width)
//        return;

    if (!clearlines)
        clearlines = BASEVIDHEIGHT; //refresh all

    // calc view size based on original game resolution
    baseviewwidth  = gr_viewwidth/vid.fdupx; //(cv_viewsize.value * BASEVIDWIDTH/10)&~7;

    baseviewheight = gr_viewheight/vid.fdupy;
    top  = gr_baseviewwindowy/vid.fdupy;
    side = gr_viewwindowx/vid.fdupx;

    // top
    HWR_DrawFlatFill (0, 0,
                     BASEVIDWIDTH, (top<clearlines ? top : clearlines),
                     st_borderflat_num);

    // left
    if (top<clearlines)
        HWR_DrawFlatFill (0, top,
                         side, (clearlines-top < baseviewheight ? clearlines-top : baseviewheight),
                         st_borderflat_num);

    // right
    if (top<clearlines)
        HWR_DrawFlatFill (side + baseviewwidth, top,
                         side, (clearlines-top < baseviewheight ? clearlines-top : baseviewheight),
                         st_borderflat_num);

    // bottom
    if (top+baseviewheight<clearlines)
        HWR_DrawFlatFill (0, top+baseviewheight,
                         BASEVIDWIDTH, BASEVIDHEIGHT,
                         st_borderflat_num);

    //
    // draw the view borders
    //

    basewindowx = (BASEVIDWIDTH - baseviewwidth)>>1;
    if (baseviewwidth==BASEVIDWIDTH)
        basewindowy = 0;
    else
        basewindowy = top;

    // top edge
    if (clearlines > basewindowy-8) {
        mpatch = W_CachePatchNum (viewborderlump[BRDR_T],PU_CACHE);
        for (x=0 ; x<baseviewwidth; x+=8)
            HWR_DrawPatch (mpatch, basewindowx+x, basewindowy-8, 0);
    }

    // bottom edge
    if (clearlines > basewindowy+baseviewheight) {
        mpatch = W_CachePatchNum (viewborderlump[BRDR_B],PU_CACHE);
        for (x=0 ; x<baseviewwidth ; x+=8)
            HWR_DrawPatch (mpatch, basewindowx+x, basewindowy+baseviewheight, 0);
    }

    // left edge
    if (clearlines > basewindowy) {
        mpatch = W_CachePatchNum (viewborderlump[BRDR_L],PU_CACHE);
        for (y=0 ; y<baseviewheight && (basewindowy+y < clearlines); y+=8)
            HWR_DrawPatch (mpatch, basewindowx-8, basewindowy+y, 0);
    }

    // right edge
    if (clearlines > basewindowy) {
        mpatch = W_CachePatchNum (viewborderlump[BRDR_R],PU_CACHE);
        for (y=0 ; y<baseviewheight && (basewindowy+y < clearlines); y+=8)
            HWR_DrawPatch (mpatch, basewindowx+baseviewwidth, basewindowy+y, 0);
    }

    // Draw beveled corners.
    if (clearlines > basewindowy-8)
        HWR_DrawPatch (W_CachePatchNum (viewborderlump[BRDR_TL],PU_CACHE),
                       basewindowx-8,
                       basewindowy-8,0);

    if (clearlines > basewindowy-8)
        HWR_DrawPatch (W_CachePatchNum (viewborderlump[BRDR_TR],PU_CACHE),
                       basewindowx+baseviewwidth,
                       basewindowy-8,0);

    if (clearlines > basewindowy+baseviewheight)
        HWR_DrawPatch (W_CachePatchNum (viewborderlump[BRDR_BL],PU_CACHE),
                       basewindowx-8,
                       basewindowy+baseviewheight,0);

    if (clearlines > basewindowy+baseviewheight)
        HWR_DrawPatch (W_CachePatchNum (viewborderlump[BRDR_BR],PU_CACHE),
                       basewindowx+baseviewwidth,
                       basewindowy+baseviewheight,0);
}


// ==========================================================================
//                                                     AM_MAP.C DRAWING STUFF
// ==========================================================================

// Clear the automap part of the screen
void HWR_clearAutomap( void )
{
    RGBA_float_t fColor = { 0,0,0,1 };

    //FIXTHIS faB - optimize by clearing only colors ?
    //HWD.pfnSetBlend ( PF_NoOcclude );

    // minx,miny,maxx,maxy
    HWD.pfnGClipRect( 0, 0, vid.width , vid.height - stbar_height, NEAR_CLIP_DIST );
    HWD.pfnClearBuffer( true, true, &fColor );
    HWD.pfnGClipRect( 0, 0, vid.width, vid.height, NEAR_CLIP_DIST );
}


// -----------------+
// HWR_drawAMline   : draw a line of the automap (the clipping is already done in automap code)
// -----------------+
//  color : palette index
void HWR_drawAMline( fline_t* fl, int color )
{
    v2d_t  v1, v2;
    RGBA_t    color_rgba;

    color_rgba = V_GetColor( color );

    v1.x = ((float)fl->a.x - vid.fx_center) * vid.fx_scale2;
    v1.y = ((float)fl->a.y - vid.fy_center) * vid.fy_scale2;

    v2.x = ((float)fl->b.x - vid.fx_center) * vid.fx_scale2;
    v2.y = ((float)fl->b.y - vid.fy_center) * vid.fy_scale2;

    HWD.pfnDraw2DLine( &v1, &v2, color_rgba );
}


// -----------------+
// HWR_DrawVidFill     : draw flat coloured rectangle, with no texture
// -----------------+
// Scaled to vid, (0,0) at upper left
//  x, y : scaled screen coord.
//  color : palette index
void HWR_DrawVidFill( int x, int y, int w, int h, int color )
{
    vxtx3d_t  v[4];
    FSurfaceInfo_t Surf;

//  3--2
//  | /|
//  |/ |
//  0--1
    v[0].x = v[3].x = (x - vid.fx_center) * vid.fx_scale2;
    v[2].x = v[1].x = ((x+w) - vid.fx_center) * vid.fx_scale2;
    v[0].y = v[1].y = (vid.fy_center - y) * vid.fy_scale2;
    v[2].y = v[3].y = (vid.fy_center - (y+h)) * vid.fy_scale2;

#ifdef _GLIDE_ARGB_
    //Hurdler: do we still use this argb color? if not, we should remove it
    v[0].argb = v[1].argb = v[2].argb = v[3].argb = 0xff00ff00; //;
#endif
    v[0].z = v[1].z = v[2].z = v[3].z = 1.0f;

    v[0].sow = v[3].sow = 0.0f;
    v[2].sow = v[1].sow = 1.0f;
    v[0].tow = v[1].tow = 0.0f;
    v[2].tow = v[3].tow = 1.0f;

    Surf.FlatColor = V_GetColor( color );

    HWD.pfnDrawPolygon( &Surf, v, 4,
                        PF_Modulated|PF_NoTexture| PF_NoDepthTest );
}


// --------------------------------------------------------------------------
// screen shot
// --------------------------------------------------------------------------
extern consvar_t cv_screenshotdir;
extern char *startupwadfiles[MAX_WADFILES];

boolean HWR_Screenshot (char *lbmname)
{
    int     i, off;
    byte*   bufw;
    uint16_t* bufr;
    byte*   dest;
    uint16_t  rgb565;

    bufr = malloc(vid.width*vid.height*2);
    if (!bufr)
        goto done;
    bufw = malloc(vid.width*vid.height*3);
    if (!bufw)
    {
        free(bufr);
        goto done;
    }

    //returns 16bit 565 RGB
    HWD.pfnReadRect (0, 0, vid.width, vid.height, vid.width*2, bufr);

    for (dest = bufw,i=0; i<vid.width*vid.height; i++)
    {
        rgb565 = bufr[i];
        *(dest++) = (rgb565 & 31) <<3;
        *(dest++) = ((rgb565 >> 5) & 63) <<2;
        *(dest++) = ((rgb565 >> 11) & 31) <<3;
    }
    free(bufr);

    if (cv_screenshotdir.string[0]) // Hurdler: Jive's request (save file in other directory)
    {
#ifdef WIN32
        char sep = '\\';
#else
        char sep = '/';
#endif
        int i;
        char wadname[MAX_WADPATH];
        char *shortwadname = NULL;
        strcpy(wadname, "DOOM");
        for (i=1; startupwadfiles[i]; i++) // seach the first "real" wad file (also skip iwad).
        {
            char *wadfile = startupwadfiles[i];
            int pos = strlen(wadfile) - 4;
            if ((pos >= 0) && !strncmp(&wadfile[pos], ".wad", 4))
            {
                strcpy(wadname, wadfile);
                wadname[pos] = '\0';
                break;
            }
        }
        shortwadname = strrchr(wadname, sep);
        sprintf(lbmname, "%s%c%s0000.tga", cv_screenshotdir.string, sep, shortwadname ? shortwadname : wadname);
    }
    else
    {
        strcpy(lbmname,"DOOM0000.tga");
    }
    off = strlen(lbmname) - 8;
    // find a file name to save it to
    for (i=0 ; i<10000; i++)
    {
        lbmname[off + 0] = '0' + ((i/1000) % 10);
        lbmname[off + 1] = '0' + ((i/100) % 10);
        lbmname[off + 2] = '0' + ((i/10) % 10);
        lbmname[off + 3] = '0' + ((i/1) % 10);
        if (access(lbmname,0) == -1) // file doesn't exist, save the file
        {
            saveTGA(lbmname, vid.width, vid.height, (GLRGB *)bufw);
            free(bufw);
            return true;
        }
    }
    free(bufw);
done:
    return false;
}


// --------------------------------------------------------------------------
// save screenshots with TGA format
// --------------------------------------------------------------------------
void saveTGA(char *file_name, int width, int height, GLRGB *buffer)
{
    int fd;
    long size;
    TGAHeader_t tga_hdr;

    fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0666);
    if (fd < 0)
        return;

    memset(&tga_hdr, 0, sizeof(tga_hdr));
    // TGA format is little-endian
    tga_hdr.width = LE_SWAP16(width);
    tga_hdr.height = LE_SWAP16(height);
    tga_hdr.image_pix_size = 24;
    tga_hdr.image_type = 2;
    tga_hdr.image_descriptor = 32;
    size = (long)width * (long)height * 3L;

    write(fd, &tga_hdr, sizeof(TGAHeader_t));
    write(fd, buffer, size);
    close(fd);
}
