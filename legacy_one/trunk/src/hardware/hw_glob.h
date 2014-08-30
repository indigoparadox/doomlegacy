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
// $Log: hw_glob.h,v $
// Revision 1.15  2001/08/12 22:08:40  hurdler
// Add alpha value for 3d water
//
// Revision 1.14  2001/08/09 21:35:23  hurdler
// Add translucent 3D water in hw mode
//
// Revision 1.13  2001/05/16 21:21:15  bpereira
// Revision 1.12  2000/11/18 15:51:25  bpereira
// Revision 1.11  2000/11/04 16:23:44  bpereira
// Revision 1.10  2000/11/02 19:49:39  bpereira
// Revision 1.9  2000/09/21 16:45:11  bpereira
//
// Revision 1.8  2000/04/27 17:48:47  hurdler
// colormap code in hardware mode is now the default
//
// Revision 1.7  2000/04/24 15:46:34  hurdler
// Support colormap for text
//
// Revision 1.6  2000/04/23 16:19:52  bpereira
// Revision 1.5  2000/04/22 21:08:23  hurdler
//
// Revision 1.4  2000/04/22 16:09:14  hurdler
// support skin color in hardware mode
//
// Revision 1.3  2000/03/29 19:39:49  bpereira
// Revision 1.2  2000/02/27 00:42:11  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:33  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//      globals (shared data & code) for hw_ modules
//
//-----------------------------------------------------------------------------

#ifndef HW_GLOB_H
#define HW_GLOB_H

#include "hw_defs.h"
#include "hw_main.h"
#include "hw_poly.h"

// the original aspect ratio of Doom graphics isn't square
#define ORIGINAL_ASPECT     (320.0f/200.0f)


// needed for sprite rendering
// equivalent of the software renderer's vissprites
typedef struct gr_vissprite_s
{
    // Doubly linked list
    struct gr_vissprite_s* prev;
    struct gr_vissprite_s* next;
    float               x1;
    float               x2;
    float               tz;
    float               ty;
    int                 patchlumpnum;
    boolean             flip;
    unsigned char       translucency;       //alpha level 0-255
    unsigned char       sectorlight;        // ...
    mobj_t              *mobj; 
   //Hurdler: 25/04/2000: now support colormap in hardware mode
    byte                *colormap;
} gr_vissprite_t;


// --------
// hw_bsp.c
// --------
extern  poly_subsector_t*   poly_subsectors;
extern  int                 addsubsector;

void HWR_InitPolyPool (void);
void HWR_FreePolyPool (void);


// --------
// hw_cache.c
// --------
void HWR_InitTextureCache (void);
void HWR_FreeTextureCache (void);
void HWR_Free_poly_subsectors (void);

void HWR_GetFlat (int flatlumpnum);
MipTexture_t * HWR_GetTexture (int tex, uint32_t drawflags);
void HWR_GetPatch (MipPatch_t* gpatch);
void HWR_GetMappedPatch(MipPatch_t* gpatch, byte *colormap);
MipPatch_t * HWR_GetPic (int lumpnum);
void HWR_SetPalette( RGBA_t *palette );

// --------
// hw_draw.c
// --------
extern  float   gr_patch_scalex;
extern  float   gr_patch_scaley;

void HWR_InitFog (void);
void HWR_FreeFog (void);
void HWR_FoggingOn (void);

extern  consvar_t cv_grrounddown;   //on/off

extern int patchformat;
extern int textureformat;
#if 0
// [WDJ] Unused
extern byte *gr_colormap; 
#endif
extern float gr_viewz;

// ------------
// misc externs
// ------------

#endif // HW_GLOB_H