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
// $Log: p_setup.h,v $
// Revision 1.6  2002/07/26 15:21:36  hurdler
//
// Revision 1.5  2002/07/24 19:03:06  ssntails
// Added support for things to retain spawned Z position.
//
// Revision 1.4  2001/01/25 22:15:43  bpereira
// added heretic support
//
// Revision 1.3  2000/11/02 17:50:08  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//   Setup a game, startup stuff.
//
//-----------------------------------------------------------------------------

#ifndef P_SETUP_H
#define P_SETUP_H

#include "doomtype.h"
#include "doomdata.h"
  // mapthing_t
#include "g_state.h"
  // skill_t
#include "r_defs.h"
  // m_fixed.h, subsector_t

// Player spawn spots for deathmatch.
#define MAX_DM_STARTS   64
extern  mapthing_t      *deathmatchstarts[MAX_DM_STARTS];
extern  int             numdmstarts;
//extern  mapthing_t**    deathmatch_p;

// MAP used flats lookup table
//
typedef struct
{
    char        name[8];        // resource name from wad
    int         lumpnum;        // lump number of the flat

    // for flat animation
    int         baselumpnum;
    int         animseq;        // start pos. in the anim sequence
    int         numpics;
    int         speed;
} levelflat_t;

extern unsigned int    numlevelflats;
extern levelflat_t*    levelflats;

int P_AddLevelFlat (char* flatname);
int P_PrecacheLevelFlats (void);
char *P_FlatNameForNum(int num);

extern int             nummapthings;
extern mapthing_t*     mapthings;

// From P_SetupLevel
extern int    level_lumpnum;  // for info and comparative savegame
extern char*  level_mapname;  // to savegame and info

// NOT called by W_Ticker. Fixme.
//  to_episode : change to episode num
//  to_map : change to map number
//  to_skill : change to skill
//  map_wadname : map command, load wad file
boolean P_SetupLevel (int      to_episode,
                      int      to_map,
                      skill_t  to_skill,
                      char*    map_wadname);


typedef struct {
  char *  mapname;  // map in string format
  byte    episode;  // episode number
  byte    map;      // map number
} level_id_t;

// Add a wadfile to the active wad files,
// replace sounds, musics, patches, textures, sprites and maps
//
//  wadfilename : filename of wad to be loaded 
//  firstmap_out : /*OUT*/  info about the first level map
boolean P_AddWadFile (char* wadfilename, /*OUT*/ level_id_t * firstmap_out );

subsector_t* R_PointInSubsector(fixed_t x, fixed_t y);


extern boolean  newlevel;
extern boolean  doom1level;

#endif
