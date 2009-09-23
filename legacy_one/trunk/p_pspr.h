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
// $Log: p_pspr.h,v $
// Revision 1.3  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//  Sprite animation.
//
//-----------------------------------------------------------------------------


#ifndef __P_PSPR__
#define __P_PSPR__

// Basic data types.
// Needs fixed point, and BAM angles.
#include "m_fixed.h"
#include "tables.h"


//
// Needs to include the precompiled
//  sprite animation tables.
// Header generated by multigen utility.
// This includes all the data for thing animation,
// i.e. the Thing Atrributes table
// and the Frame Sequence table.
#include "info.h"

#ifdef __GNUG__
#pragma interface
#endif


//
// Frame flags:
// handles maximum brightness (torches, muzzle flare, light sources)
//

// faB: I noticed they didn't use the 32 bits of the frame field,
//      so now we use the upper 16 bits for new effects.

#define FF_FRAMEMASK    0x7fff  // only the frame number
#define FF_FULLBRIGHT   0x8000  // frame always appear full bright (fixedcolormap)

// faB:
//  MF_SHADOW is no more used to activate translucency (or the old fuzzy)
//  The frame field allows to set translucency per frame, instead of per sprite.
//  Now, (frame & FF_TRANSMASK) is the translucency table number, if 0
//  it is not translucent.

// Note:
//  MF_SHADOW still affects the targeting for monsters (they miss more)

#define FF_TRANSMASK   0x70000  // 0 = no trans(opaque), 1-7 = transl. table
#define FF_TRANSSHIFT       16

// faB: new 'alpha' shade effect, for smoke..

#define FF_SMOKESHADE  0x80000  // sprite is an alpha channel



// translucency tables

// TODO: add another asm routine which use the fg and bg indexes in the
//       inverse order so the 20-80 becomes 80-20 translucency, no need
//       for other tables (thus 1090,2080,5050,8020,9010, and fire special)

typedef enum
{
    tr_transmed=1,   //sprite 50 backg 50  most shots
    tr_transmor=2,   //       20       80  puffs
    tr_transhi =3,   //       10       90  blur effect
    tr_transfir=4,   // 50 50 but brighter for fireballs, shots..
    tr_transfx1=5    // 50 50 brighter some colors, else opaque for torches
} transnum_t;


//
// Overlay psprites are scaled shapes
// drawn directly on the view screen,
// coordinates are given for a 320*200 view screen.
//
typedef enum
{
    ps_weapon,
    ps_flash,
    NUMPSPRITES

} psprnum_t;

typedef struct
{
    state_t*    state;  // a NULL state means not active
    int         tics;
    fixed_t     sx;
    fixed_t     sy;

} pspdef_t;

void P_OpenWeapons(void);
void P_CloseWeapons(void);

#endif
