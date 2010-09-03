// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2010 by DooM Legacy Team.
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
// $Log: doomstat.h,v $
// Revision 1.14  2003/07/23 17:20:37  darkwolf95
// Initial Chex Quest 1 Support
//
// Revision 1.13  2002/12/13 22:34:27  ssntails
// MP3/OGG support!
//
// Revision 1.12  2001/07/16 22:35:40  bpereira
// - fixed crash of e3m8 in heretic
// - fixed crosshair not drawed bug
//
// Revision 1.11  2001/04/17 22:26:07  calumr
// Initial Mac add
//
// Revision 1.10  2001/04/01 17:35:06  bpereira
// no message
//
// Revision 1.9  2001/02/24 13:35:19  bpereira
// no message
//
// Revision 1.8  2001/01/25 22:15:41  bpereira
// added heretic support
//
// Revision 1.7  2000/11/02 17:50:06  stroggonmeth
// Big 3Dfloors & FraggleScript commit!!
//
// Revision 1.6  2000/10/21 08:43:28  bpereira
// no message
//
// Revision 1.5  2000/08/31 14:30:55  bpereira
// no message
//
// Revision 1.4  2000/08/10 19:58:04  bpereira
// no message
//
// Revision 1.3  2000/08/10 14:53:10  ydario
// OS/2 port
//
// Revision 1.2  2000/02/27 00:42:10  hurdler
// fix CR+LF problem
//
// Revision 1.1.1.1  2000/02/22 20:32:32  hurdler
// Initial import into CVS (v1.29 pr3)
//
//
// DESCRIPTION:
//   All the global variables that store the internal state.
//   Theoretically speaking, the internal state of the engine
//    should be found by looking at the variables collected
//    here, and every relevant module will have to include
//    this header file.
//   In practice, things are a bit messy.
//
//-----------------------------------------------------------------------------


#ifndef __D_STATE__
#define __D_STATE__


// We need globally shared data structures,
//  for defining the global state variables.
#include "doomdata.h"

// We need the player data structure as well.
#include "d_player.h"
#include "d_clisrv.h"


// Game mode handling - identify IWAD version,
//  handle IWAD dependend animations etc.
typedef enum
{
    shareware,    // DOOM 1 shareware, E1, M9
    registered,   // DOOM 1 registered, E3, M27
    commercial,   // DOOM 2 retail, E1 M34
    // FreeDoom is DOOM 2, can play as commercial or indetermined
    // DOOM 2 german edition not handled
    retail,       // DOOM 1 retail, E4, M36
    heretic,
    hexen,
    strife,
    indetermined, // Well, no IWAD found.
    chexquest1	  // DarkWolf95:July 14, 2003: Chex Quest Support

} gamemode_e;


// Mission packs - might be useful for TC stuff?
typedef enum
{
    doom,         // DOOM 1
    doom2,        // DOOM 2
    pack_tnt,     // TNT mission pack
    pack_plut,    // Plutonia pack
    mission_none

} gamemission_t;


// Identify language to use, software localization.
typedef enum
{
    english,
    french,
    german,
    lang_unknown

} language_t;

// [WDJ] Structure some of the scattered game differences.
typedef struct
{
    char * 	gname;	       // game name, used in savegame
    char *	startup_title; // startup page
    char *	idstr;	       // used for directory and command line
    char * 	iwad_filename; // doom, doom2, heretic, heretic1, hexen, etc.
    char *	support_wad;   // another wad to support the game
    const char * keylump[2];    // required lump names
    byte	require_lump;  // lumps that must appear (bit set)
    byte	reject_lump;   // lumps that must not appear (bit set)
    uint16_t	gameflags;     // assorted flags
    gamemode_e	gamemode;
} game_desc_t;

enum gameflags_e {
   GD_idwad       = 0x01, // one of the commercial/shareware wads by id or Raven
   GD_iwad_pref   = 0x02, // load the iwad after legacy.wad to give it preference
   GD_unsupported = 0x08, // unsupported game type
};

// Index to game_desc_t entries
// This is also the search order.
typedef enum {
    GDESC_freedoom,
    GDESC_freedm,
    GDESC_doom2,
    GDESC_freedoom_ultimate,
    GDESC_ultimate,
    GDESC_ultimate_se,
    GDESC_doom,
    GDESC_doom_shareware,
    GDESC_plutonia,
    GDESC_tnt,
    GDESC_blasphemer,
    GDESC_heretic,
    GDESC_heretic_shareware,
    GDESC_hexen,
    GDESC_hexen_demo,
    GDESC_strife,
    GDESC_strife_shareware,
    GDESC_chex1,
    GDESC_ultimate_mode,
    GDESC_doom_mode,
    GDESC_heretic_mode,
    GDESC_hexen_mode,
    GDESC_other, // other iwad entry, and table search limit
    GDESC_num	// number of entries in game_desc_table
} game_desc_e;

// ===================================================
// Game Mode - identify IWAD as shareware, retail etc.
// ===================================================
//
extern game_desc_e     gamedesc_index;  // game_desc_table index, unique game id
extern game_desc_t     gamedesc;	// active desc used by most of legacy
extern gamemode_e      gamemode;
extern gamemission_t   gamemission;
extern boolean         inventory;   // true with heretic and hexen
extern boolean         raven;       // true with heretic and hexen

// Set if homebrew PWAD stuff has been added.
extern  boolean	       modifiedgame;


// =========
// Language.
// =========
//
extern  language_t   language;



// =============================
// Selected skill type, map etc.
// =============================

// Selected by user.
extern  skill_t         gameskill;	// easy, medium, hard
extern  byte            gameepisode;	// Doom episode, 1..4
extern  byte            gamemap;	// level 1..32

// Nightmare mode flag, single player.
// extern  boolean         respawnmonsters;

// Netgame? only true in a netgame
extern  boolean         netgame;
// Only true if >1 player. netgame => multiplayer but not (multiplayer=>netgame)
extern  boolean         multiplayer;

// Flag: true only if started as net deathmatch.
// An enum might handle altdeath/cooperative better.
extern  consvar_t       cv_deathmatch;


// ========================================
// Internal parameters for sound rendering.
// ========================================

extern boolean         nomusic; //defined in d_main.c
extern boolean         nosound;

// =========================
// Status flags for refresh.
// =========================
//

// Depending on view size - no status bar?
// Note that there is no way to disable the
//  status bar explicitely.
extern  boolean statusbaractive;

extern  boolean menuactive;     // Menu overlayed?
extern  boolean paused;         // Game Pause?

extern  boolean         nodrawers;
extern  boolean         noblit;

extern  int             viewwindowx;
extern  int             viewwindowy;
extern  int             rdraw_viewheight;		// was viewheight
extern  int             rdraw_viewwidth;		// was viewwidth
extern  int             rdraw_scaledviewwidth;		// was scaledrviewwidth



// This one is related to the 3-screen display mode.
// ANG90 = left side, ANG270 = right
extern  int     viewangleoffset;

// Player taking events, and displaying.
extern  int     consoleplayer;
extern  int     displayplayer;
extern  int     secondarydisplayplayer; // for splitscreen

//added:16-01-98: player from which the statusbar displays the infos.
extern  int     statusbarplayer;


// ============================================
// Statistics on a given map, for intermission.
// ============================================
//
extern  int     totalkills;
extern  int     totalitems;
extern  int     totalsecret;


// ===========================
// Internal parameters, fixed.
// ===========================
// These are set by the engine, and not changed
//  according to user inputs. Partly load from
//  WAD, partly set at startup time.

extern  tic_t           gametic;
#ifdef CLIENTPREDICTION2
extern  tic_t           localgametic;
#else
#define localgametic  leveltime
#endif

// Player spawn spots.
extern  mapthing_t      *playerstarts[MAXPLAYERS];

// Intermission stats.
// Parameters for world map / intermission.
extern  wbstartstruct_t         wminfo;


// LUT of ammunition limits for each kind.
// This doubles with BackPack powerup item.
extern  int             maxammo[NUMAMMO];





// =====================================
// Internal parameters, used for engine.
// =====================================
//

// File handling stuff.
extern  char            basedefault[1024];

#ifdef __MACOS__
#define DEBFILE(msg) I_OutputMsg(msg)
extern  FILE*           debugfile;
#else
#define DEBUGFILE
#ifdef DEBUGFILE
#define DEBFILE(msg) { if(debugfile) fputs(msg,debugfile); }
extern  FILE*           debugfile;
#else
#define DEBFILE(msg) {}
extern  FILE*           debugfile;
#endif
#endif //__MACOS__


// if true, load all graphics at level load
extern  boolean         precache;


// wipegamestate can be set to -1
//  to force a wipe on the next draw
extern  gamestate_t     wipegamestate;

//?
// debug flag to cancel adaptiveness
extern  boolean         singletics;

#define   BODYQUESIZE     32

extern mobj_t*   bodyque[BODYQUESIZE];
extern  int             bodyqueslot;


// =============
// Netgame stuff
// =============


//extern  ticcmd_t        localcmds[BACKUPTICS];

extern  ticcmd_t        netcmds[BACKUPTICS][MAXPLAYERS];

#endif //__D_STATE__
